#include <fstream>

#include <efx-creative.h>
#include <EFX-Util.h>

#define WriteEffect(_effect_)\
  EAXREVERBPROPERTIES _effect_##props = _effect_;\
  ConvertReverbParameters(&_effect_##props, &efxReverb);\
  out << "EFXREVERBPROPERTIES " #_effect_ " = {";\
  out << "(float)" << efxReverb.flDensity << ", ";\
  out << "(float)" << efxReverb.flDiffusion << ", ";\
  out << "(float)" << efxReverb.flGain << ", ";\
  out << "(float)" << efxReverb.flGainHF << ", ";\
  out << "(float)" << efxReverb.flGainLF << ", ";\
  out << "(float)" << efxReverb.flDecayTime << ", ";\
  out << "(float)" << efxReverb.flDecayHFRatio << ", ";\
  out << "(float)" << efxReverb.flDecayLFRatio << ", ";\
  out << "(float)" << efxReverb.flReflectionsGain << ", ";\
  out << "(float)" << efxReverb.flReflectionsDelay << ", ";\
  out << "(float)" << efxReverb.flReflectionsPan[0] << ", ";\
  out << "(float)" << efxReverb.flReflectionsPan[1] << ", ";\
  out << "(float)" << efxReverb.flReflectionsPan[2] << ", ";\
  out << "(float)" << efxReverb.flLateReverbGain << ", ";\
  out << "(float)" << efxReverb.flLateReverbDelay << ", ";\
  out << "(float)" << efxReverb.flLateReverbPan[0] << ", ";\
  out << "(float)" << efxReverb.flLateReverbPan[1] << ", ";\
  out << "(float)" << efxReverb.flLateReverbPan[2] << ", ";\
  out << "(float)" << efxReverb.flEchoTime << ", ";\
  out << "(float)" << efxReverb.flEchoDepth << ", ";\
  out << "(float)" << efxReverb.flModulationTime << ", ";\
  out << "(float)" << efxReverb.flModulationDepth << ", ";\
  out << "(float)" << efxReverb.flAirAbsorptionGainHF << ", ";\
  out << "(float)" << efxReverb.flHFReference << ", ";\
  out << "(float)" << efxReverb.flLFReference << ", ";\
  out << "(float)" << efxReverb.flRoomRolloffFactor << ", ";\
  out << efxReverb.iDecayHFLimit;\
  out << "};\n\n"\

int main(int argc, char** argv){
  std::ofstream out("EFXeffects.h");

  out << 
"typedef struct\n"
"{\n"
"	float flDensity;\n"
"	float flDiffusion;\n"
"	float flGain;\n"
"	float flGainHF;\n"
"	float flGainLF;\n"
"	float flDecayTime;\n"
"	float flDecayHFRatio;\n"
"	float flDecayLFRatio;\n"
"	float flReflectionsGain;\n"
"	float flReflectionsDelay;\n"
"	float flReflectionsPan[3];\n"
"	float flLateReverbGain;\n"
"	float flLateReverbDelay;\n"
"	float flLateReverbPan[3];\n"
"	float flEchoTime;\n"
"	float flEchoDepth;\n"
"	float flModulationTime;\n"
"	float flModulationDepth;\n"
"	float flAirAbsorptionGainHF;\n"
"	float flHFReference;\n"
"	float flLFReference;\n"
"	float flRoomRolloffFactor;\n"
"	int	iDecayHFLimit;\n"
"} EFXREVERBPROPERTIES;\n"
"\n";

  EFXEAXREVERBPROPERTIES efxReverb;

  WriteEffect(REVERB_PRESET_PADDEDCELL);
  WriteEffect(REVERB_PRESET_LIVINGROOM);
  WriteEffect(REVERB_PRESET_CAVE);
  WriteEffect(REVERB_PRESET_CARPETTEDHALLWAY);
  WriteEffect(REVERB_PRESET_ALLEY);
  WriteEffect(REVERB_PRESET_MOUNTAINS);
  WriteEffect(REVERB_PRESET_PARKINGLOT);
  WriteEffect(REVERB_PRESET_DRUGGED);
  WriteEffect(REVERB_PRESET_CONCERTHALL);
  WriteEffect(REVERB_PRESET_ROOM);
  WriteEffect(REVERB_PRESET_STONEROOM);
  WriteEffect(REVERB_PRESET_ARENA);
  WriteEffect(REVERB_PRESET_HALLWAY);
  WriteEffect(REVERB_PRESET_FOREST);
  WriteEffect(REVERB_PRESET_QUARRY);
  WriteEffect(REVERB_PRESET_SEWERPIPE);
  WriteEffect(REVERB_PRESET_DIZZY);
  WriteEffect(REVERB_PRESET_BATHROOM);
  WriteEffect(REVERB_PRESET_AUDITORIUM);
  WriteEffect(REVERB_PRESET_HANGAR);
  WriteEffect(REVERB_PRESET_STONECORRIDOR);
  WriteEffect(REVERB_PRESET_CITY);
  WriteEffect(REVERB_PRESET_PLAIN);
  WriteEffect(REVERB_PRESET_UNDERWATER);
  WriteEffect(REVERB_PRESET_PSYCHOTIC);

  out.close();
  return 0;
}
