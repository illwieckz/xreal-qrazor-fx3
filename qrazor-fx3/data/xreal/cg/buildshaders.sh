#!/bin/sh

#
# ARB shaders
#
cgc -profile arbvp1 generic_arbvp1.cg -o generic_arbvp1.s
cgc -profile arbfp1 generic_arbfp1.cg -o generic_arbfp1.s

cgc -profile arbvp1 zfill_arbvp1.cg -o zfill_arbvp1.s
cgc -profile arbfp1 zfill_arbfp1.cg -o zfill_arbfp1.s

cgc -profile arbvp1 lighting_R_arbvp1.cg -o lighting_R_arbvp1.s
cgc -profile arbfp1 lighting_R_arbfp1.cg -o lighting_R_arbfp1.s

cgc -profile arbvp1 lighting_R_arbvp1.cg -o lighting_R_arbvp1.s
cgc -profile arbfp1 lighting_R_arbfp1.cg -o lighting_R_arbfp1.s

cgc -profile arbvp1 lighting_RB_arbvp1.cg -o lighting_RB_arbvp1.s
cgc -profile arbfp1 lighting_RB_arbfp1.cg -o lighting_RB_arbfp1.s

cgc -profile arbvp1 lighting_RBH_arbvp1.cg -o lighting_RBH_arbvp1.s
cgc -profile arbfp1 lighting_RBH_arbfp1.cg -o lighting_RBH_arbfp1.s

cgc -profile arbvp1 lighting_RBHS_arbvp1.cg -o lighting_RBHS_arbvp1.s
cgc -profile arbfp1 lighting_RBHS_arbfp1.cg -o lighting_RBHS_arbfp1.s

cgc -profile arbvp1 lighting_RBS_arbvp1.cg -o lighting_RBS_arbvp1.s
cgc -profile arbfp1 lighting_RBS_arbfp1.cg -o lighting_RBS_arbfp1.s

cgc -profile arbvp1 lighting_D_XY_Z_CUBE_arbvp1.cg -o lighting_D_XY_Z_CUBE_arbvp1.s
cgc -profile arbfp1 lighting_D_XY_Z_CUBE_arbfp1.cg -o lighting_D_XY_Z_CUBE_arbfp1.s

cgc -profile arbvp1 lighting_D_XY_Z_proj_arbvp1.cg -o lighting_D_XY_Z_proj_arbvp1.s
cgc -profile arbfp1 lighting_D_XY_Z_proj_arbfp1.cg -o lighting_D_XY_Z_proj_arbfp1.s

cgc -profile arbvp1 lighting_DB_XY_Z_CUBE_arbvp1.cg -o lighting_DB_XY_Z_CUBE_arbvp1.s
cgc -profile arbfp1 lighting_DB_XY_Z_CUBE_arbfp1.cg -o lighting_DB_XY_Z_CUBE_arbfp1.s

cgc -profile arbvp1 lighting_DBH_XY_Z_CUBE_arbvp1.cg -o lighting_DBH_XY_Z_CUBE_arbvp1.s
cgc -profile arbfp1 lighting_DBH_XY_Z_CUBE_arbfp1.cg -o lighting_DBH_XY_Z_CUBE_arbfp1.s

cgc -profile arbvp1 lighting_DBHS_XY_Z_CUBE_arbvp1.cg -o lighting_DBHS_XY_Z_CUBE_arbvp1.s
cgc -profile arbfp1 lighting_DBHS_XY_Z_CUBE_arbfp1.cg -o lighting_DBHS_XY_Z_CUBE_arbfp1.s

cgc -profile arbvp1 lighting_DBS_XY_Z_CUBE_arbvp1.cg -o lighting_DBS_XY_Z_CUBE_arbvp1.s
cgc -profile arbfp1 lighting_DBS_XY_Z_CUBE_arbfp1.cg -o lighting_DBS_XY_Z_CUBE_arbfp1.s

cgc -profile arbvp1 reflection_C_arbvp1.cg -o reflection_C_arbvp1.s
cgc -profile arbfp1 reflection_C_arbfp1.cg -o reflection_C_arbfp1.s

cgc -profile arbvp1 refraction_C_arbvp1.cg -o refraction_C_arbvp1.s
cgc -profile arbfp1 refraction_C_arbfp1.cg -o refraction_C_arbfp1.s

cgc -profile arbvp1 dispersion_C_arbvp1.cg -o dispersion_C_arbvp1.s
cgc -profile arbfp1 dispersion_C_arbfp1.cg -o dispersion_C_arbfp1.s

#cgc -profile arbvp1 liquid_C_arbvp1.cg -o liquid_C_arbvp1.s
#cgc -profile arbfp1 liquid_C_arbfp1.cg -o liquid_C_arbfp1.s


#
# NV30 shaders
#
cgc -profile vp30 generic_vp30.cg -o generic_vp30.s
cgc -profile fp30 generic_fp30.cg -o generic_fp30.s

cgc -profile vp30 zfill_vp30.cg -o zfill_vp30.s
cgc -profile fp30 zfill_fp30.cg -o zfill_fp30.s

cgc -profile vp30 lighting_R_vp30.cg -o lighting_R_vp30.s
cgc -profile fp30 lighting_R_fp30.cg -o lighting_R_fp30.s

cgc -profile vp30 lighting_RB_vp30.cg -o lighting_RB_vp30.s
cgc -profile fp30 lighting_RB_fp30.cg -o lighting_RB_fp30.s

cgc -profile vp30 lighting_RBH_vp30.cg -o lighting_RBH_vp30.s
cgc -profile fp30 lighting_RBH_fp30.cg -o lighting_RBH_fp30.s

cgc -profile vp30 lighting_RBHS_vp30.cg -o lighting_RBHS_vp30.s
cgc -profile fp30 lighting_RBHS_fp30.cg -o lighting_RBHS_fp30.s

cgc -profile vp30 lighting_RBS_vp30.cg -o lighting_RBS_vp30.s
cgc -profile fp30 lighting_RBS_fp30.cg -o lighting_RBS_fp30.s

cgc -profile vp30 lighting_D_XY_Z_CUBE_vp30.cg -o lighting_D_XY_Z_CUBE_vp30.s
cgc -profile fp30 lighting_D_XY_Z_CUBE_fp30.cg -o lighting_D_XY_Z_CUBE_fp30.s

cgc -profile vp30 lighting_DB_XY_Z_CUBE_vp30.cg -o lighting_DB_XY_Z_CUBE_vp30.s
cgc -profile fp30 lighting_DB_XY_Z_CUBE_fp30.cg -o lighting_DB_XY_Z_CUBE_fp30.s

cgc -profile vp30 lighting_DBH_XY_Z_CUBE_vp30.cg -o lighting_DBH_XY_Z_CUBE_vp30.s
cgc -profile fp30 lighting_DBH_XY_Z_CUBE_fp30.cg -o lighting_DBH_XY_Z_CUBE_fp30.s

cgc -profile vp30 lighting_DBHS_XY_Z_CUBE_vp30.cg -o lighting_DBHS_XY_Z_CUBE_vp30.s
cgc -profile fp30 lighting_DBHS_XY_Z_CUBE_fp30.cg -o lighting_DBHS_XY_Z_CUBE_fp30.s

cgc -profile vp30 lighting_DBS_XY_Z_CUBE_vp30.cg -o lighting_DBS_XY_Z_CUBE_vp30.s
cgc -profile fp30 lighting_DBS_XY_Z_CUBE_fp30.cg -o lighting_DBS_XY_Z_CUBE_fp30.s

cgc -profile vp30 reflection_C_vp30.cg -o reflection_C_vp30.s
cgc -profile fp30 reflection_C_fp30.cg -o reflection_C_fp30.s

cgc -profile vp30 refraction_C_vp30.cg -o refraction_C_vp30.s
cgc -profile fp30 refraction_C_fp30.cg -o refraction_C_fp30.s

cgc -profile vp30 dispersion_C_vp30.cg -o dispersion_C_vp30.s
cgc -profile fp30 dispersion_C_fp30.cg -o dispersion_C_fp30.s


