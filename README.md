# linux-initrd-initbase
This is a base init program for initramfs (early init). It does nothing except
mounting some kernel virtual file systems (like /dev, /proc, /sys etc.) and
preparing the current console TTY.

## Build
You need `musl-gcc` to statically build this program with musl library.
Also make sure you have `make` installed on your system. Run `make` to
build this project.

## License
linux-initrd-initbase, Open-Source & Public Domain. 2026 ubelstahl. Check
[LICENSE](./LICENSE) for more information.

## Mentioned Projects
### [musl](https://musl.libc.org/)
Copyright © 2005-2020 Rich Felker, et al.
Open-Source, MIT License. Check license at
[musl/COPYRIGHT](https://git.musl-libc.org/cgit/musl/tree/COPYRIGHT)

### [Busybox](https://busybox.net/)
Open-Source, GPL2 License. Get more information at
[busybox.net/license.html](https://busybox.net/license.html)