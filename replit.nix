{ pkgs }: {
	deps = [
   pkgs.fuse-emulator
   pkgs.kmod
   pkgs.sshfs
   pkgs.openssh
		pkgs.clang
		pkgs.ccls
		pkgs.gdb
		pkgs.gnumake
		pkgs.gmp
		pkgs.gmpxx
	];
}