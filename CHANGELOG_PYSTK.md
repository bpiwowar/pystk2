# Changelog

This file documents notable changes to PySTK2 across versions since its inception.

## Version 0.5.0

- Track: access to the track graph (successors)
- Kart: access to the current track index (node)

## Version 0.4.0

- Kart state: aux tick state (for acceleration boost)

## Version 0.3.7

- Fix camera bugs

## Version 0.3.6

- Name are displayed on top of the karts

## Version 0.3.5

- `num_cameras` can be used to setup cameras that follow the first karts

## Version 0.3.4

- More flexible camera
- Collected kart energy
- Position of the kart in the race

## Version 0.3.3

- feature: get the action of a given kart

## Version 0.3.2

- New kart information: skidding factor
- Fix bug when no player exists (i.e. supertuxkart has never been launched)

## Version 0.3.1

- Bug fix (crash with some races)
- List tracks with a given mode

## Version 0.3.0

- Fix bug when the controlled kart is not the first
- Breaking change: quaternion is now [w, x, y, z] (and not the non standard [x, y, z, w])
- Use numpy arrays rather than python lists

## Version 0.2.0

- Fix with cameras: now set to the number of controlled players
- Includes world state phase to detect when the race starts
- Prevents graphic driver initialization when running in no graphics mode

## Version 0.1.0

- Initial release based on a (manual) merge of PySTK and STK branch 1.4
