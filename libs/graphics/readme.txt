
Use as follows :

setBgColorRGB(0, 0, 255);	// blue screen
initScreen();	// Call DisplaySetFrameBuf & clear screen

setPrintColorRGB(255, 0, 0);
PRTSTR0("Text");
cls();	// clear screen
PRTSTR1("Var: %08lX", var);
PRTSTR2("String: %s, address %08lX", name, name);

...


need to set DisplaySetFrameBuf function pointer as extern somewhere in code for initScreen to work

