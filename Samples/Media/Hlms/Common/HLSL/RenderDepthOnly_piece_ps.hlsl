
@property( !hlms_render_depth_only )
	@property( !hlms_shadowcaster )
		@piece( output_type )float4@end
	@end @property( hlms_shadowcaster )
		@piece( output_type )float@end
	@end
	@piece( output_type_sv ): SV_Target0@end
@end @property( hlms_render_depth_only )
	@piece( output_type )void@end
@end

@property( hlms_render_depth_only && !alpha_test )
	@set( hlms_disable_stage, 1 )
@end
