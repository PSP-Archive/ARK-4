Here is the source of inerno - a reversed and improved march33 driver.


Inferno Version 2 (ARK-4) changes:

- Fixed bug in sceUmdCheckMedium that would return true even when no ISO file was set. This removes the need to have a fake.cso on Vita.
- Changed static patches to isofs.prx with dynamic ones for cross-device compatibility.
- Changed ISO Cache to allways be enabled with minimum cache. Allows for ISO Cache to be used on PSP 1K and fixes many reading bugs.
