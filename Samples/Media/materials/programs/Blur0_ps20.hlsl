sampler RT: register(s0);
// Simple blur filter

float4 main(float2 texCoord: TEXCOORD0) : COLOR {

	float2 samples[12] = {
   	-0.326212, -0.405805,
   	-0.840144, -0.073580,
   	-0.695914,  0.457137,
   	-0.203345,  0.620716,
    	0.962340, -0.194983,
    	0.473434, -0.480026,
    	0.519456,  0.767022,
    	0.185461, -0.893124,
    	0.507431,  0.064425,
    	0.896420,  0.412458,
   	-0.321940, -0.932615,
   	-0.791559, -0.597705,
	};

   float4 sum = tex2D(RT, texCoord);
   for (int i = 0; i < 12; i++){
      sum += tex2D(RT, texCoord + 0.025 * samples[i]);
   }
   return sum / 13;

}




