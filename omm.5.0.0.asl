// --------------------------------------------------------------------
// LiveSplit Auto-Splitter script for Odyssey Mario's Moveset speedruns
// By PeachyPeach
// --------------------------------------------------------------------
// Starts when the player selects a file
// Splits each time a certain amount of stars is collected
// Stops when the Grand Star is grabbed
// Resets when the option "Return to Main Menu" is selected
// Also resets after leaving the cake ending screen
// --------------------------------------------------------------------
// Edit the "omm/omm_splits.txt" file with your intended splits
// For instance, if your splits are 10 stars, 20 stars and 30 stars,
// write "10 20 30" to the file (the Grand star is not needed)
// --------------------------------------------------------------------
// You need omm.5.0.0.patch or later to make it work
// Make sure the executable you play is named "sm64.us.f3dex2e.exe"
// --------------------------------------------------------------------

state("sm64.us.f3dex2e") {
}

init {
	vars.shouldStart = 0;
	vars.shouldSplit = 0;
	vars.shouldReset = 0;
	vars.dataAddr = 0;
	byte[] buffer = memory.ReadBytes(modules.First().BaseAddress, modules.First().ModuleMemorySize);
	for (int i = 0; i != -1;) {
		i = Array.IndexOf(buffer, (byte) 0x4F, i + 1);
		if (i != -1 &&
			buffer[i + 0x0] == 0x4F &&
			buffer[i + 0x1] == 0x4D &&
			buffer[i + 0x2] == 0x4D &&
			buffer[i + 0x3] == 0x41 &&
			buffer[i + 0x4] == 0x55 &&
			buffer[i + 0x5] == 0x54 &&
			buffer[i + 0x6] == 0x4F &&
			buffer[i + 0x7] == 0x53 &&
			buffer[i + 0x8] == 0x50 &&
			buffer[i + 0x9] == 0x4C &&
			buffer[i + 0xA] == 0x49 &&
			buffer[i + 0xB] == 0x54) {
			vars.dataAddr = i;
			print("OMMAUTOSPLIT found at address " + vars.dataAddr.ToString());
			break;
		}
	}
}

update {
	vars.shouldStart = memory.ReadValue<byte>((IntPtr) ((long) modules.First().BaseAddress + (long) vars.dataAddr + 0xC));
	vars.shouldSplit = memory.ReadValue<byte>((IntPtr) ((long) modules.First().BaseAddress + (long) vars.dataAddr + 0xD));
	vars.shouldReset = memory.ReadValue<byte>((IntPtr) ((long) modules.First().BaseAddress + (long) vars.dataAddr + 0xE));
	memory.WriteValue<byte>((IntPtr) ((long) modules.First().BaseAddress + (long) vars.dataAddr + 0xC), 0);
	memory.WriteValue<byte>((IntPtr) ((long) modules.First().BaseAddress + (long) vars.dataAddr + 0xD), 0);
	memory.WriteValue<byte>((IntPtr) ((long) modules.First().BaseAddress + (long) vars.dataAddr + 0xE), 0);
	return true;
}

start {
	return (vars.shouldStart == 1);
}

split {
	return (vars.shouldSplit == 1);
}

reset {
	return (vars.shouldReset == 1);
}
