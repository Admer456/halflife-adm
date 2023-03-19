
# Vehicle system

## Introduction
A long time ago, I had a dream about being in a Half-Life map with a nice canyon and some cliffs, and a helipad. I entered an old Trabant and it had funny bouncy suspension. I drove it uphill and reached the helipad with 2 drivable helicopters, which I crashed into. It was nice, and then I woke up.

Time to make that into reality.

## Prerequisites
- [Physics](Physics.md) is needed, otherwise this will be as janky as CS 1.6's `func_vehicle`
- Lots of utilities for animations & bones, so we can look up bones by name etc.

## Scope
- Simple 4-wheeled vehicles
- Simple flying vehicles
- Simple water vehicles
- Weaponised vehicles
- Brush-based vehicles

## Implementation

Vehicle entity classnames would be prefixed with `veh_`.

### Phase 1
- Engine component - does stuff with RPM, torque etc., calculates the motor force
- Wheel component - contains a wheel phys body and is influenced by the motor
- Body component - maintains vehicle health, contains all other components
- Seat component - lets players sit there; is associated either with a bone or an offset from the entity's origin
- Basic 4-wheeled vehicle

### Phase 2
- Basic flying vehicle
- Basic water vehicle
- Weapon component - can be used by players on associated gunner seats
- Decls for vehicle parametrisation

### Phase 3
- Brush-based vehicles
- Traditional `func_vehicle` for fun

## Gains
We'd have very fun vehicles to play with and run other players over. Black Mesa Derby, anyone?

Also:  
FUNC_VEHICLE IS A RIGHT, NOT A PRIVILEGE
