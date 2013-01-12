!!NVfp4.0                                        
TEMP texcoord, c0, c1, frac;                     
MOV texcoord, fragment.texcoord[0];              
FLR texcoord.z, texcoord;                        
TEX c0, texcoord, texture[0], ARRAY2D;           
ADD texcoord.z, texcoord, { 0, 0, 1, 0 };        
TEX c1, texcoord, texture[0], ARRAY2D;           
FRC frac.x, fragment.texcoord[0].z;              
LRP result.color, frac.x, c1, c0;                
END
