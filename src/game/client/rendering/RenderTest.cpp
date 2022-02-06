
#include "hud.h"


#include "SDL2/SDL.h"

// Fix problems with HSPRITE SDK definitions conflicting with Windows header definitions
// Man, we should just mass-rename HSPRITE to SpriteHandle or something like that... 
#define WINDOWS_IGNORE_PACKING_MISMATCH 1
#define HSPRITE WINDOWS_HSPRITE
#include "SDL2/SDL_syswm.h"
#undef HSPRITE

// Hack: on Linux, some symbols are already defined by XLib, so undefine them
#ifdef Bool
#undef Bool
#endif
#ifdef True
#undef True
#endif
#ifdef False
#undef False
#endif

// DiligentCore targets already include the interfaces for us
// But I prefer to be a little more verbose
#include "Graphics/GraphicsEngineVulkan/interface/EngineFactoryVk.h"

#include "Graphics/GraphicsEngine/interface/RenderDevice.h"
#include "Graphics/GraphicsEngine/interface/DeviceContext.h"
#include "Graphics/GraphicsEngine/interface/SwapChain.h"
#include "Graphics/GraphicsTools/interface/MapHelper.hpp"

#include "Common/interface/RefCntAutoPtr.hpp"

namespace Experiment
{

	namespace ShaderSources
	{
	static constexpr const char* Vertex = R"(
	
	#define g_Time (g_Misc.x)
	
	cbuffer Constants
	{
	    float4 g_Misc;
	};
	
	struct VSInput
	{
	    float3 Pos : ATTRIB0;
	};
	
	struct PSInput 
	{ 
	    float4 Pos : SV_POSITION;
	    float Time;
	};
	
	void main( in  VSInput VSIn,
	           out PSInput PSIn ) 
	{
	    PSIn.Pos.x = VSIn.Pos.x 
	        + sin( g_Time + (VSIn.Pos.x * 0.5) + (VSIn.Pos.y * 0.234) ) * 0.1
	        + sin( g_Time * 3.333 + VSIn.Pos.y * 4.66 ) * 0.05;
	    
	    PSIn.Pos.y = VSIn.Pos.y 
	        + sin( g_Time + (VSIn.Pos.y * 0.7) ) * 0.1
	        + sin( (g_Time * 0.25) * (VSIn.Pos.x * 4.0) ) * 0.06;
	
	    PSIn.Pos.z = VSIn.Pos.z;
	    PSIn.Pos.w = 1.0;
	
	    PSIn.Time = g_Time;
	}
	)";
	
	static constexpr const char* Fragment = R"(
	
	struct PSInput 
	{ 
	    float4 Pos : SV_POSITION;
	    float Time;
	};
	
	struct PSOutput
	{ 
	    float4 Color : SV_TARGET; 
	};
	
	float nsin( float x )
	{
	    return (sin(x * 0.0067) * 0.5) + 0.5;
	}
	
	// Note that if separate shader objects are not supported (this is only the case for old GLES3.0 devices), vertex
	// shader output variable name must match exactly the name of the pixel shader input variable.
	// If the variable has structure type (like in this example), the structure declarations must also be identical.
	void main( in  PSInput  PSIn,
	           out PSOutput PSOut )
	{
	    // Let's be special and make sum blobs ^w^
	    PSOut.Color.x = nsin(PSIn.Pos.x + PSIn.Time * 32.5);
	    PSOut.Color.y = nsin(PSIn.Pos.y + PSIn.Time * (32.5 + sin(PSIn.Time * 0.4) * 10.0));
	    PSOut.Color.z = nsin(PSIn.Pos.x + PSIn.Pos.y + PSIn.Time * 32.5);
	    PSOut.Color.w = 1.0;
	}
	)";
	}


	// Let's draw a pentagon, as opposed to a triangle that everyone does lol
	const Vector PentagonVertices[] =
	{
		{0.0f, 0.5f, 0.0f},
		{0.5f, 0.2f, 0.0f},
		{0.3f, -0.4f, 0.0f},
		{-0.3f, -0.4f, 0.0f},
		{-0.5f, 0.2f, 0.0f}
	};

	const int PentagonIndices[] =
	{
		0, 1, 2,
		0, 2, 3,
		0, 3, 4
	};

	// Generic constant buffer member
	struct CBufferElement
	{
		float x, y, z, w;
	};

	constexpr int Width = 800;
	constexpr int Height = 600;
	constexpr int CenterPos = SDL_WINDOWPOS_CENTERED;

	Diligent::NativeWindow GetNativeWindow( SDL_Window* window )
	{
		Diligent::NativeWindow nativeWindow;

		SDL_SysWMinfo wmInfo;
		SDL_VERSION(&wmInfo.version);
		SDL_GetWindowWMInfo(window, &wmInfo);

#ifdef WIN32
		nativeWindow.hWnd = wmInfo.info.win.window;
#else
		// SDL2 only does X11
		nativeWindow.WindowId = wmInfo.info.x11.window;
		nativeWindow.pDisplay = wmInfo.info.x11.display;
		nativeWindow.pXCBConnection = nullptr;
#endif
		return nativeWindow;
	}
	static SDL_Window* diligentWindow{nullptr};

	static Diligent::RefCntAutoPtr<Diligent::IRenderDevice> device;
	static Diligent::RefCntAutoPtr<Diligent::IDeviceContext> immediateContext;
	static Diligent::RefCntAutoPtr<Diligent::ISwapChain> swapChain;

	static Diligent::RefCntAutoPtr<Diligent::IPipelineState> PSO;
	static Diligent::RefCntAutoPtr<Diligent::IShaderResourceBinding> SRB;
	static Diligent::RefCntAutoPtr<Diligent::IBuffer> TriangleVertexBuffer;
	static Diligent::RefCntAutoPtr<Diligent::IBuffer> TriangleIndexBuffer;
	static Diligent::RefCntAutoPtr<Diligent::IBuffer> VSConstants;

	void InitDiligentEngine()
	{
		// Errr... the version of SDL2 that ships with Half-Life doesn't quite... uh, have Vulkan
		// Let's hope this works? Otherwise resort to that one SDL2 hack Solo mentioned
		diligentWindow = SDL_CreateWindow("DiligentCore in Half-Life", CenterPos, CenterPos, Width, Height, SDL_WINDOW_OPENGL);

		using namespace Diligent;

		// Swap chain descriptor
		// A swap chain is basically a buncha screen buffers
		SwapChainDesc SCDesc;
		SCDesc.Width = Width;
		SCDesc.Height = Height;

		// Attributes specific to Vulkan engine, use default
		EngineVkCreateInfo EngineCI;

		// Obtain the Vulkan engine, create device and context from EngineCI
		auto* factory = GetEngineFactoryVk();
		factory->CreateDeviceAndContextsVk(EngineCI, &device, &immediateContext);

		if (nullptr == device)
		{
			Con_Printf("DiligentEngineExperiment: device == nullptr\n");
			return;
		}

		if (nullptr == immediateContext)
		{
			Con_Printf("DiligentEngineExperiment: immediateContext == nullptr\n");
			return;
		}

		// Create a swap chain for our current SDL2 window
		// (a double-buffer swap chain in this instance, because of SCDesc defaults)
		NativeWindow nativeWindow = GetNativeWindow(diligentWindow);
		factory->CreateSwapChainVk(device, immediateContext, SCDesc, nativeWindow, &swapChain);

		if (nullptr == swapChain)
		{
			Con_Printf("DiligentEngineExperiment: swapChain == nullptr\n");
			return;
		}
		else 
		{
			Con_Printf("DiligentEngineExperiment: success\n");
		}

		float shaderTime{0.0f};

		// ====================================
		// 1. Pipeline stuff

		// Pipeline state object info
		GraphicsPipelineStateCreateInfo PSOCI;
		PSOCI.PSODesc.Name = "Triangle PSO";
		PSOCI.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;
		PSOCI.GraphicsPipeline.NumRenderTargets = 1;
		PSOCI.GraphicsPipeline.RTVFormats[0] = swapChain->GetDesc().ColorBufferFormat;
		PSOCI.GraphicsPipeline.DSVFormat = swapChain->GetDesc().DepthBufferFormat;
		PSOCI.GraphicsPipeline.PrimitiveTopology = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		PSOCI.GraphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_NONE; // for now, don't cull anything
		PSOCI.GraphicsPipeline.DepthStencilDesc.DepthEnable = true;

		// ====================================
		// 1.1. Shader stuff

		// Shader info
		ShaderCreateInfo ShaderCI;
		ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;
		//ShaderCI.UseCombinedTextureSamplers = true; // OpenGL needs this, even tho' we don't use OpenGL 'round 'ere

		// Create a vertex shader
		// Notice how ShaderCI will be reused for the fragment shader too
		RefCntAutoPtr<IShader> vertexShader;
		{
			ShaderCI.Desc.Name = "Triangle vertex shader";
			ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
			ShaderCI.EntryPoint = "main";
			ShaderCI.Source = ShaderSources::Vertex;
			device->CreateShader(ShaderCI, &vertexShader);
			{
				// Dynamic buffer for time
				BufferDesc CBDesc;
				CBDesc.Name = "Triangle vertex shader constant buffer";
				CBDesc.Size = sizeof(float) * 4;
				CBDesc.Usage = USAGE_DYNAMIC;
				CBDesc.BindFlags = BIND_UNIFORM_BUFFER;
				CBDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
				device->CreateBuffer(CBDesc, nullptr, &VSConstants);
			}
		}

		// Create a fragment/pixel shader
		RefCntAutoPtr<IShader> fragmentShader;
		{
			ShaderCI.Desc.Name = "Triangle fragment shader";
			ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
			ShaderCI.EntryPoint = "main";
			ShaderCI.Source = ShaderSources::Fragment;
			device->CreateShader(ShaderCI, &fragmentShader);
		}

		PSOCI.pVS = vertexShader;
		PSOCI.pPS = fragmentShader;

		PSOCI.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

		// ====================================
		// 1.2. Layout and binding stuff

		// Must define the data layout
		LayoutElement LayoutElements[] =
		{
			LayoutElement{0, 0, 3, VT_FLOAT32, false}
		};

		PSOCI.GraphicsPipeline.InputLayout.LayoutElements = LayoutElements;
		PSOCI.GraphicsPipeline.InputLayout.NumElements = 1;

		// Finally, create the pipeline state object
		device->CreateGraphicsPipelineState(PSOCI, &PSO);

		int numShaderVariables = PSO->GetStaticVariableCount(SHADER_TYPE_VERTEX);
		int numShaderResources = vertexShader->GetResourceCount();

		// Since we did not explcitly specify the type for 'Constants' variable, default
		// type (SHADER_RESOURCE_VARIABLE_TYPE_STATIC) will be used. Static variables never
		// change and are bound directly through the pipeline state object.
		auto* shaderConstantsVar = PSO->GetStaticVariableByName(SHADER_TYPE_VERTEX, "Constants");
		if (nullptr != shaderConstantsVar)
		{
			shaderConstantsVar->Set(VSConstants);
		}
		else
		{
			static bool saidIt = false;

			if (!saidIt)
			{
				std::cout << "I can't find 'g_Misc', only found " << numShaderVariables << " other variables"
						  << " and " << numShaderResources << " shader resources" << std::endl;
				saidIt = true;
			}
		}

		// Create a shader resource binding object and bind all static resources in it
		PSO->CreateShaderResourceBinding(&SRB, true);

		// ====================================
		// 2. Geometry stuff

		BufferDesc VBDesc;
		VBDesc.Name = "Triangle VB";
		VBDesc.Usage = USAGE_IMMUTABLE;
		VBDesc.BindFlags = BIND_VERTEX_BUFFER;
		VBDesc.Size = sizeof(PentagonVertices);

		BufferData VBData;
		VBData.pData = PentagonVertices;
		VBData.DataSize = VBDesc.Size;

		device->CreateBuffer(VBDesc, &VBData, &TriangleVertexBuffer);

		// ====================================
		// 2.1. Index stuff

		BufferDesc IBDesc;
		IBDesc.Name = "Triangle IB";
		IBDesc.Usage = USAGE_IMMUTABLE;
		IBDesc.BindFlags = BIND_INDEX_BUFFER;
		IBDesc.Size = sizeof(PentagonIndices);

		BufferData IBData;
		IBData.pData = PentagonIndices;
		IBData.DataSize = IBDesc.Size;

		device->CreateBuffer(IBDesc, &IBData, &TriangleIndexBuffer);
	}

	static const float ClearColour[]{0.01f, 0.05f, 0.05f, 1.00f};

	void UpdateDiligentEngine(float frameTime)
	{
		using namespace Diligent;

		auto* RTV = swapChain->GetCurrentBackBufferRTV();
		auto* DSV = swapChain->GetDepthBufferDSV();
		immediateContext->SetRenderTargets(1, &RTV, DSV, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

		// Clear the screen
		immediateContext->ClearRenderTarget(RTV, ClearColour, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
		immediateContext->ClearDepthStencil(DSV, CLEAR_DEPTH_FLAG, 1.0f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

		SDL_Event ev;
		while (SDL_PollEvent(&ev))
		{
			if (ev.type == SDL_QUIT)
			{
				return;
			}
		}

		// Map the buffer and write current shader time
		MapHelper<CBufferElement> shaderConstants(immediateContext, VSConstants, MAP_WRITE, MAP_FLAG_DISCARD);
		*shaderConstants = {gHUD.m_flTime, 0.0f, 0.0f, 0.0f};
	
		// ====================================
		// 3.1. Binding le bufferz and doing a drawcall

		const uint64_t Offset = 0;
		IBuffer* buffers[]{TriangleVertexBuffer};

		immediateContext->SetVertexBuffers(0, 1, buffers, &Offset, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);
		immediateContext->SetIndexBuffer(TriangleIndexBuffer, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

		// Since we can have multiple pipeline states
		immediateContext->SetPipelineState(PSO);

		// Committing shader resources is like updating shader variables
		immediateContext->CommitShaderResources(SRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
	
		// Indexed draw call description
		DrawIndexedAttribs drawAttributes;
		drawAttributes.IndexType = VT_UINT32;
		drawAttributes.NumIndices = sizeof(PentagonIndices) / sizeof(int);
		drawAttributes.Flags = DRAW_FLAG_VERIFY_ALL;

		immediateContext->DrawIndexed(drawAttributes);

		swapChain->Present();
	}

}
