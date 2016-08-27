@property( !hlms_render_depth_only )
	@piece( output_type )PS_OUTPUT@end
	@piece( PsOutputDecl )
		struct PS_OUTPUT
		{
		@property( !hlms_shadowcaster )
			float4 colour0 [[ color(0) ]];
		@end @property( hlms_shadowcaster )
			float colour0 [[ color(0) ]];
		@end
		};
	@end
@end @property( hlms_render_depth_only )
	@piece( output_type )void@end
	@piece( PsOutputDecl )
		struct PS_OUTPUT
		{
		@property( !hlms_shadowcaster )
			float4 colour0;
		@end @property( hlms_shadowcaster )
			float colour0;
		@end
		};
	@end
@end

@property( hlms_render_depth_only && !alpha_test )
	@set( hlms_disable_stage, 1 )
@end
