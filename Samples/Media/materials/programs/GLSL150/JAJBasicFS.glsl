#version 150

out vec4 fragColour;

uniform vec4 ColourMe[2] = vec4[](vec4(0,0,0,0),vec4(0,0,1,0));

layout(binding = 0) uniform atomic_uint atom_counter;
// layout(binding = 0) uniform atomic_uint atom_counter2;


// Pixel shader
void main()
{
  // for (int i = 0; i < 3; i++) {
  //     uint counter = atomicCounterIncrement(atom_counter[i]);
  // }
  uint counter = atomicCounterIncrement(atom_counter);
  // uint counter2 = atomicCounterIncrement(atom_counter2);
  // uint counter4eva = atomicCounterIncrement(atom_counter[1]);

  fragColour = ColourMe[0] + ColourMe[1] + counter / 4E9;
  // fragColour = vec4(1, 1, 1, 1);
}
