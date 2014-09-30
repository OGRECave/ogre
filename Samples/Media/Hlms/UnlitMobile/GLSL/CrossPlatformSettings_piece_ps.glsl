@piece( SetCrossPlatformSettings )
@property( GL3+ )
#version 330
#define FRAG_COLOR		0
layout(location = FRAG_COLOR, index = 0) out vec4 outColour;
	@piece( texture2D )texture@end
	@piece( textureCube )texture@end
	@piece( textureCubeLod )textureLod@end

	@piece( lowp )highp@end
	@piece( mediump )highp@end
@end @property( !GL3+ )
#define in varying
#define outColour gl_FragColor
	@property( hlms_high_quality )
		@piece( lowp )highp@end
		@piece( mediump )highp@end
		precision highp float;
	@end @property( !hlms_high_quality )
		@piece( lowp )lowp@end
		@piece( mediump )mediump@end
		precision mediump float;
	@end

	@piece( texture2D )texture2D@end
	@piece( textureCube )textureCube@end
	@piece( textureCubeLod )textureCube@end
@end
@end
