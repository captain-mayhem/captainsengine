typedef struct
{
	float flDensity;
	float flDiffusion;
	float flGain;
	float flGainHF;
	float flGainLF;
	float flDecayTime;
	float flDecayHFRatio;
	float flDecayLFRatio;
	float flReflectionsGain;
	float flReflectionsDelay;
	float flReflectionsPan[3];
	float flLateReverbGain;
	float flLateReverbDelay;
	float flLateReverbPan[3];
	float flEchoTime;
	float flEchoDepth;
	float flModulationTime;
	float flModulationDepth;
	float flAirAbsorptionGainHF;
	float flHFReference;
	float flLFReference;
	float flRoomRolloffFactor;
	int	iDecayHFLimit;
} EFXREVERBPROPERTIES;

EFXREVERBPROPERTIES REVERB_PRESET_PADDEDCELL = {(float)0.1715, (float)1, (float)0.316228, (float)0.001, (float)1, (float)0.17, (float)0.1, (float)1, (float)0.250035, (float)0.001, (float)0, (float)0, (float)0, (float)1.26911, (float)0.002, (float)0, (float)0, (float)0, (float)0.25, (float)0, (float)0.25, (float)0, (float)0.99426, (float)5000, (float)250, (float)0, 1};

EFXREVERBPROPERTIES REVERB_PRESET_LIVINGROOM = {(float)0.976563, (float)1, (float)0.316228, (float)0.001, (float)1, (float)0.5, (float)0.1, (float)1, (float)0.205116, (float)0.003, (float)0, (float)0, (float)0, (float)0.280543, (float)0.004, (float)0, (float)0, (float)0, (float)0.25, (float)0, (float)0.25, (float)0, (float)0.99426, (float)5000, (float)250, (float)0, 1};

EFXREVERBPROPERTIES REVERB_PRESET_CAVE = {(float)1, (float)1, (float)0.316228, (float)1, (float)1, (float)2.91, (float)1.3, (float)1, (float)0.500035, (float)0.015, (float)0, (float)0, (float)0, (float)0.706318, (float)0.022, (float)0, (float)0, (float)0, (float)0.25, (float)0, (float)0.25, (float)0, (float)0.99426, (float)5000, (float)250, (float)0, 0};

EFXREVERBPROPERTIES REVERB_PRESET_CARPETTEDHALLWAY = {(float)0.428687, (float)1, (float)0.316228, (float)0.01, (float)1, (float)0.3, (float)0.1, (float)1, (float)0.121479, (float)0.002, (float)0, (float)0, (float)0, (float)0.153109, (float)0.03, (float)0, (float)0, (float)0, (float)0.25, (float)0, (float)0.25, (float)0, (float)0.99426, (float)5000, (float)250, (float)0, 1};

EFXREVERBPROPERTIES REVERB_PRESET_ALLEY = {(float)1, (float)0.3, (float)0.316228, (float)0.732825, (float)1, (float)1.49, (float)0.86, (float)1, (float)0.250035, (float)0.007, (float)0, (float)0, (float)0, (float)0.995405, (float)0.011, (float)0, (float)0, (float)0, (float)0.125, (float)0.95, (float)0.25, (float)0, (float)0.99426, (float)5000, (float)250, (float)0, 1};

EFXREVERBPROPERTIES REVERB_PRESET_MOUNTAINS = {(float)1, (float)0.27, (float)0.316228, (float)0.0562341, (float)1, (float)1.49, (float)0.21, (float)1, (float)0.040738, (float)0.3, (float)0, (float)0, (float)0, (float)0.191867, (float)0.1, (float)0, (float)0, (float)0, (float)0.25, (float)1, (float)0.25, (float)0, (float)0.99426, (float)5000, (float)250, (float)0, 0};

EFXREVERBPROPERTIES REVERB_PRESET_PARKINGLOT = {(float)1, (float)1, (float)0.316228, (float)1, (float)1, (float)1.65, (float)1.5, (float)1, (float)0.208209, (float)0.008, (float)0, (float)0, (float)0, (float)0.265155, (float)0.012, (float)0, (float)0, (float)0, (float)0.25, (float)0, (float)0.25, (float)0, (float)0.99426, (float)5000, (float)250, (float)0, 0};

EFXREVERBPROPERTIES REVERB_PRESET_DRUGGED = {(float)0.428687, (float)0.5, (float)0.316228, (float)1, (float)1, (float)8.39, (float)1.39, (float)1, (float)0.875992, (float)0.002, (float)0, (float)0, (float)0, (float)3.10814, (float)0.03, (float)0, (float)0, (float)0, (float)0.25, (float)0, (float)0.25, (float)1, (float)0.99426, (float)5000, (float)250, (float)0, 0};

EFXREVERBPROPERTIES REVERB_PRESET_CONCERTHALL = {(float)1, (float)1, (float)0.316228, (float)0.562341, (float)1, (float)3.92, (float)0.7, (float)1, (float)0.242661, (float)0.02, (float)0, (float)0, (float)0, (float)0.9977, (float)0.029, (float)0, (float)0, (float)0, (float)0.25, (float)0, (float)0.25, (float)0, (float)0.99426, (float)5000, (float)250, (float)0, 1};

EFXREVERBPROPERTIES REVERB_PRESET_ROOM = {(float)0.428687, (float)1, (float)0.316228, (float)0.592925, (float)1, (float)0.4, (float)0.83, (float)1, (float)0.150314, (float)0.002, (float)0, (float)0, (float)0, (float)1.06292, (float)0.003, (float)0, (float)0, (float)0, (float)0.25, (float)0, (float)0.25, (float)0, (float)0.99426, (float)5000, (float)250, (float)0, 1};

EFXREVERBPROPERTIES REVERB_PRESET_STONEROOM = {(float)1, (float)1, (float)0.316228, (float)0.707946, (float)1, (float)2.31, (float)0.64, (float)1, (float)0.441062, (float)0.012, (float)0, (float)0, (float)0, (float)1.10027, (float)0.017, (float)0, (float)0, (float)0, (float)0.25, (float)0, (float)0.25, (float)0, (float)0.99426, (float)5000, (float)250, (float)0, 1};

EFXREVERBPROPERTIES REVERB_PRESET_ARENA = {(float)1, (float)1, (float)0.316228, (float)0.447713, (float)1, (float)7.24, (float)0.33, (float)1, (float)0.261216, (float)0.02, (float)0, (float)0, (float)0, (float)1.01859, (float)0.03, (float)0, (float)0, (float)0, (float)0.25, (float)0, (float)0.25, (float)0, (float)0.99426, (float)5000, (float)250, (float)0, 1};

EFXREVERBPROPERTIES REVERB_PRESET_HALLWAY = {(float)0.3645, (float)1, (float)0.316228, (float)0.707946, (float)1, (float)1.49, (float)0.59, (float)1, (float)0.245754, (float)0.007, (float)0, (float)0, (float)0, (float)1.6615, (float)0.011, (float)0, (float)0, (float)0, (float)0.25, (float)0, (float)0.25, (float)0, (float)0.99426, (float)5000, (float)250, (float)0, 1};

EFXREVERBPROPERTIES REVERB_PRESET_FOREST = {(float)1, (float)0.3, (float)0.316228, (float)0.0223872, (float)1, (float)1.49, (float)0.54, (float)1, (float)0.0524807, (float)0.162, (float)0, (float)0, (float)0, (float)0.768245, (float)0.088, (float)0, (float)0, (float)0, (float)0.125, (float)1, (float)0.25, (float)0, (float)0.99426, (float)5000, (float)250, (float)0, 1};

EFXREVERBPROPERTIES REVERB_PRESET_QUARRY = {(float)1, (float)1, (float)0.316228, (float)0.316228, (float)1, (float)1.49, (float)0.83, (float)1, (float)0, (float)0.061, (float)0, (float)0, (float)0, (float)1.77828, (float)0.025, (float)0, (float)0, (float)0, (float)0.125, (float)0.7, (float)0.25, (float)0, (float)0.99426, (float)5000, (float)250, (float)0, 1};

EFXREVERBPROPERTIES REVERB_PRESET_SEWERPIPE = {(float)0.307063, (float)0.8, (float)0.316228, (float)0.316228, (float)1, (float)2.81, (float)0.14, (float)1, (float)1.6387, (float)0.014, (float)0, (float)0, (float)0, (float)3.24713, (float)0.021, (float)0, (float)0, (float)0, (float)0.25, (float)0, (float)0.25, (float)0, (float)0.99426, (float)5000, (float)250, (float)0, 1};

EFXREVERBPROPERTIES REVERB_PRESET_DIZZY = {(float)0.3645, (float)0.6, (float)0.316228, (float)0.630957, (float)1, (float)17.23, (float)0.56, (float)1, (float)0.139155, (float)0.02, (float)0, (float)0, (float)0, (float)0.493742, (float)0.03, (float)0, (float)0, (float)0, (float)0.25, (float)1, (float)0.81, (float)0.31, (float)0.99426, (float)5000, (float)250, (float)0, 0};

EFXREVERBPROPERTIES REVERB_PRESET_BATHROOM = {(float)0.1715, (float)1, (float)0.316228, (float)0.251189, (float)1, (float)1.49, (float)0.54, (float)1, (float)0.653131, (float)0.007, (float)0, (float)0, (float)0, (float)3.27341, (float)0.011, (float)0, (float)0, (float)0, (float)0.25, (float)0, (float)0.25, (float)0, (float)0.99426, (float)5000, (float)250, (float)0, 1};

EFXREVERBPROPERTIES REVERB_PRESET_AUDITORIUM = {(float)1, (float)1, (float)0.316228, (float)0.578096, (float)1, (float)4.32, (float)0.59, (float)1, (float)0.403181, (float)0.02, (float)0, (float)0, (float)0, (float)0.716968, (float)0.03, (float)0, (float)0, (float)0, (float)0.25, (float)0, (float)0.25, (float)0, (float)0.99426, (float)5000, (float)250, (float)0, 1};

EFXREVERBPROPERTIES REVERB_PRESET_HANGAR = {(float)1, (float)1, (float)0.316228, (float)0.316228, (float)1, (float)10.05, (float)0.23, (float)1, (float)0.500035, (float)0.02, (float)0, (float)0, (float)0, (float)1.25603, (float)0.03, (float)0, (float)0, (float)0, (float)0.25, (float)0, (float)0.25, (float)0, (float)0.99426, (float)5000, (float)250, (float)0, 1};

EFXREVERBPROPERTIES REVERB_PRESET_STONECORRIDOR = {(float)1, (float)1, (float)0.316228, (float)0.761202, (float)1, (float)2.7, (float)0.79, (float)1, (float)0.247172, (float)0.013, (float)0, (float)0, (float)0, (float)1.5758, (float)0.02, (float)0, (float)0, (float)0, (float)0.25, (float)0, (float)0.25, (float)0, (float)0.99426, (float)5000, (float)250, (float)0, 1};

EFXREVERBPROPERTIES REVERB_PRESET_CITY = {(float)1, (float)0.5, (float)0.316228, (float)0.398107, (float)1, (float)1.49, (float)0.67, (float)1, (float)0.0730298, (float)0.007, (float)0, (float)0, (float)0, (float)0.142725, (float)0.011, (float)0, (float)0, (float)0, (float)0.25, (float)0, (float)0.25, (float)0, (float)0.99426, (float)5000, (float)250, (float)0, 1};

EFXREVERBPROPERTIES REVERB_PRESET_PLAIN = {(float)1, (float)0.21, (float)0.316228, (float)0.1, (float)1, (float)1.49, (float)0.5, (float)1, (float)0.058479, (float)0.179, (float)0, (float)0, (float)0, (float)0.108893, (float)0.1, (float)0, (float)0, (float)0, (float)0.25, (float)1, (float)0.25, (float)0, (float)0.99426, (float)5000, (float)250, (float)0, 1};

EFXREVERBPROPERTIES REVERB_PRESET_UNDERWATER = {(float)0.3645, (float)1, (float)0.316228, (float)0.01, (float)1, (float)1.49, (float)0.1, (float)1, (float)0.596348, (float)0.007, (float)0, (float)0, (float)0, (float)7.07946, (float)0.011, (float)0, (float)0, (float)0, (float)0.25, (float)0, (float)1.18, (float)0.348, (float)0.99426, (float)5000, (float)250, (float)0, 1};

EFXREVERBPROPERTIES REVERB_PRESET_PSYCHOTIC = {(float)0.0625, (float)0.5, (float)0.316228, (float)0.840427, (float)1, (float)7.56, (float)0.91, (float)1, (float)0.486407, (float)0.02, (float)0, (float)0, (float)0, (float)2.43781, (float)0.03, (float)0, (float)0, (float)0, (float)0.25, (float)0, (float)4, (float)1, (float)0.99426, (float)5000, (float)250, (float)0, 0};

