!!ARBfp1.0
# ARB_fragment_program generated by NVIDIA Cg compiler
# cgc version 1.2.1001, build date Mar 17 2004  10:58:07
# command line args: -profile arbfp1
#vendor NVIDIA Corporation
#version 1.0.02
#profile arbfp1
#program main
#semantic main.diffusemap
#semantic main.bumpmap
#semantic main.specularmap
#semantic main.lightmap
#semantic main.deluxemap
#semantic main.view_origin
#semantic main.bump_scale
#semantic main.specular_exponent
#var sampler2D diffusemap :  : texunit 0 : 1 : 1
#var sampler2D bumpmap :  : texunit 1 : 2 : 1
#var sampler2D specularmap :  : texunit 2 : 3 : 1
#var sampler2D lightmap :  : texunit 3 : 4 : 1
#var sampler2D deluxemap :  : texunit 4 : 5 : 1
#var float3 view_origin :  : c[2] : 6 : 1
#var float bump_scale :  : c[1] : 7 : 1
#var float specular_exponent :  : c[0] : 8 : 1
#var float4 IN.position : $vin.TEXCOORD0 : TEX0 : 0 : 1
#var float4 IN.tex_diffuse_bump : $vin.TEXCOORD1 : TEX1 : 0 : 1
#var float4 IN.tex_specular : $vin.TEXCOORD2 : TEX2 : 0 : 1
#var float4 IN.tex_light : $vin.TEXCOORD3 : TEX3 : 0 : 1
#var float4 IN.tex_deluxe : $vin.TEXCOORD4 : TEX4 : 0 : 1
#var float3 IN.tangent : $vin.TEXCOORD5 : TEX5 : 0 : 1
#var float3 IN.binormal : $vin.TEXCOORD6 : TEX6 : 0 : 1
#var float3 IN.normal : $vin.TEXCOORD7 : TEX7 : 0 : 1
#var float4 main.color : $vout.COLOR : COL : -1 : 1
PARAM u2 = program.local[2];
PARAM u1 = program.local[1];
PARAM u0 = program.local[0];
PARAM c0 = {2, 0.5, 0, 0};
TEMP R0;
TEMP R1;
TEMP R2;
TEMP R3;
TEX R0.xyz, fragment.texcoord[4], texture[4], 2D;
TEX R1.xyz, fragment.texcoord[1].zwzz, texture[1], 2D;
ADD R0.xyz, R0, -c0.y;
MUL R0.xyz, R0, c0.x;
ADD R1.xyz, R1, -c0.y;
MUL R1.xyz, R1, c0.x;
ADD R2.xyz, u2, -fragment.texcoord[0];
DP3 R0.w, fragment.texcoord[5], R2;
MOV R3.x, R0.w;
DP3 R0.w, fragment.texcoord[6], R2;
MOV R3.y, R0.w;
DP3 R0.w, fragment.texcoord[7], R2;
MOV R3.z, R0.w;
DP3 R0.w, fragment.texcoord[5], R0;
MOV R2.x, R0.w;
DP3 R0.w, fragment.texcoord[6], R0;
MOV R2.y, R0.w;
DP3 R0.x, fragment.texcoord[7], R0;
MOV R2.z, R0.x;
DP3 R0.x, R3, R3;
RSQ R0.x, R0.x;
MUL R3.xyz, R0.x, R3;
MOV R0.xy, R1;
MUL R0.w, R1.z, u1.x;
MOV R0.z, R0.w;
DP3 R0.w, R2, R2;
RSQ R0.w, R0.w;
MAD R3.xyz, R0.w, R2, R3;
MUL R2.xyz, R0.w, R2;
DP3 R0.w, R0, R0;
RSQ R0.w, R0.w;
MUL R0.xyz, R0.w, R0;
DP3_SAT R0.w, R0, R2;
DP3 R1.x, R3, R3;
RSQ R1.x, R1.x;
MUL R3.xyz, R1.x, R3;
DP3_SAT R0.x, R0, R3;
POW R0.x, R0.x, u0.x;
TEX R1.xyz, fragment.texcoord[3], texture[3], 2D;
TEX R2, fragment.texcoord[1], texture[0], 2D;
MUL R3.xyz, R1, R0.w;
MUL R3.xyz, R2, R3;
MOV R2.xyz, R3;
TEX R3.xyz, fragment.texcoord[2], texture[2], 2D;
MUL R1.xyz, R3, R1;
MAD R0.xyz, R1, R0.x, R2;
MOV R2.xyz, R0;
MOV result.color, R2;
END
# 48 instructions, 4 R-regs, 0 H-regs.
# End of program
