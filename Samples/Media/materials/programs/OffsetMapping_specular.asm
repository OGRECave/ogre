// Pixel Shader for doing bump mapping with parallax plus diffuse and specular lighting by nfz 

// uv               TEXCOORD0 
// lightDir            TEXCOORD1 
// eyeDir            TEXCOORD2 
// half               TEXCOORD3 

// lightDiffuse         c0 
// lightSpecular      c1 
// Parallax scale and bias c2 
// normal/height map   texunit 0 - height map in alpha channel 
// diffuse texture      texunit 1 

ps.1.4 


texld r0, t0               // get height 
texcrd r2.xyz, t0            // get uv coordinates 
texcrd r3.xyz, t2            // get eyedir vector 


mad r0.xyz, r0.a, c2.x, c2.y   // displacement = height * scale + bias 
mad r2.xyz, r3, r0, r2         // newtexcoord = eyedir * displacement + uv 

phase 

texld r0, r2.xyz            // get normal N using newtexcoord 
texld r1, r2.xyz            // get diffuse texture using newtexcoord 
texcrd r4.xyz, t1            // get lightdir vector 
texcrd r5.xyz, t3            // get half angle vector 

dp3_sat r5.rgb, r0_bx2, r5         // N dot H - spec calc 
dp3_sat r4.rgb, r0_bx2, r4         // N dot L - diffuse calc 
+ mul r5.a, r5.r, r5.r 
mul r0.rgb, r4, r1            // colour = diffusetex * N dot L 
+ mul r5.a, r5.a, r5.a 

mul r5.rgb, r5.a, r5.a          
mul r5.rgb, r5, r5          
mul r5.rgb, r5, r5          
mul r5.rgb, r5, c1            // specular = (N dot H)^32 * specularlight 

mad r0.rgb, r0, c0, r5         // colour = diffusetex * (N dot L)* diffuselight + specular 
+ mov r0.a, c2.b 