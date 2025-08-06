#ifndef DEBUG_H
#define DEBUG_H

#ifdef DEBUG_PRINTF
//#define DEBUG_ONLY_FIRST_BOUNCE
#extension GL_EXT_debug_printf : enable
bool b = false;
bool bb = false;	/// Marker to isolate some operations

#define DP(message) \
if (b) { debugPrintfEXT(message); }
#define DP1(message, v1) \
if (b) { debugPrintfEXT(message, v1); }
#define DP2(message, v1, v2) \
if (b) { debugPrintfEXT(message, v1, v2); }
#define DP3(message, v1, v2, v3) \
if (b) { debugPrintfEXT(message, v1, v2, v3); }
#define DP4(message, v1, v2, v3, v4) \
if (b) { debugPrintfEXT(message, v1, v2, v3, v4); }
#define DP5(message, v1, v2, v3, v4, v5) \
if (b) { debugPrintfEXT(message, v1, v2, v3, v4, v5); }
#define DP6(message, v1, v2, v3, v4, v5, v6) \
if (b) { debugPrintfEXT(message, v1, v2, v3, v4, v5, v6); }
#define DP7(message, v1, v2, v3, v4, v5, v6, v7) \
if (b) { debugPrintfEXT(message, v1, v2, v3, v4, v5, v6, v7); }
#define DP8(message, v1, v2, v3, v4, v5, v6, v7, v8) \
if (b) { debugPrintfEXT(message, v1, v2, v3, v4, v5, v6, v7, v8); }
#define DPF(f1) \
if (b) { debugPrintfEXT("%f\n", f1); }
#define DPF1(f1) \
if (b) { debugPrintfEXT("%f\n", f1); }
#define DPF2(f1, f2) \
if (b) { debugPrintfEXT("%f, %f\n", f1, f2); }
#define DPF3(f1, f2, f3) \
if (b) { debugPrintfEXT("%f, %f, %f\n", f1, f2, f3); }
#define DPF4(f1, f2, f3, f4) \
if (b) { debugPrintfEXT("%f, %f, %f, %f\n", f1, f2, f3, f4); }
#define DPV2(v) \
if (b) { debugPrintfEXT("%f, %f\n", v); }
#define DPV3(v) \
if (b) { debugPrintfEXT("%f, %f, %f\n", v); }
#define DPV4(v) \
if (b) { debugPrintfEXT("%f, %f, %f, %f\n", v); }
#define MARK(I) \
if (b) { debugPrintfEXT("Mark: %i\n", I); }
#define BOOL(B, I) \
if (b && B) { debugPrintfEXT("Bool: %i\n", I); }

void DPDirection(vec3 Origin, vec3 Direction, int Color)
{
	if (b)
	{
		debugPrintfEXT("((%f, %f, %f), (%f, %f, %f), %i),\n", Origin.x, Origin.y, Origin.z, Direction.x, Direction.y, Direction.z, Color);
	}
}

void DS(int Index, vec3 Data)
{
	switch (Index)
	{
		case 0: { DebugGlobal0.xyz = Data; break;}
		case 1: { DebugGlobal1.xyz = Data; break;}
		case 2: { DebugGlobal2.xyz = Data; break;}
		case 3: { DebugGlobal3.xyz = Data; break;}
		case 4: { DebugGlobal4.xyz = Data; break;}
		case 5: { DebugGlobal5.xyz = Data; break;}
		case 6: { DebugGlobal6.xyz = Data; break;}
		case 7: { DebugGlobal7.xyz = Data; break;}
	}
}

void DS(int Index, float Data)
{
	switch (Index)
	{
		case 0: { DebugGlobal0.xyz = vec3(Data); break; }
		case 1: { DebugGlobal1.xyz = vec3(Data); break; }
		case 2: { DebugGlobal2.xyz = vec3(Data); break; }
		case 3: { DebugGlobal3.xyz = vec3(Data); break; }
		case 4: { DebugGlobal4.xyz = vec3(Data); break; }
		case 5: { DebugGlobal5.xyz = vec3(Data); break; }
		case 6: { DebugGlobal6.xyz = vec3(Data); break; }
		case 7: { DebugGlobal7.xyz = vec3(Data); break; }
	}
}
#endif

#endif