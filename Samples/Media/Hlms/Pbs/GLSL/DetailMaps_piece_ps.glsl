// detail_maps_diffuse & detail_maps_normal are either 0 or 4

@foreach( detail_maps_diffuse, n )
	@property( detail_offsetsD@n )
		@piece( offsetDetailD@n ) * material.detailOffsetScaleD[@value(currOffsetDetailD)].zw + material.detailOffsetScaleD[@counter(currOffsetDetailD)].xy@end
	@end
@end
@foreach( detail_maps_normal, n )
	@property( detail_offsetsN@n )
		@piece( offsetDetailN@n ) * material.detailOffsetScaleN[@value(currOffsetDetailN)].zw + material.detailOffsetScaleN[@counter(currOffsetDetailN)].xy@end
	@end
@end

@piece( detail_swizzle0 )x@end;
@piece( detail_swizzle1 )y@end;
@piece( detail_swizzle2 )z@end;
@piece( detail_swizzle3 )w@end;

/*
Down below we perform:
	if( detail_maps_normal )
		second_valid_detail_map_nm = first_valid_detail_map_nm + 1;
	else
		second_valid_detail_map_nm = 0;
*/
@property( detail_maps_normal )
	@add( second_valid_detail_map_nm, first_valid_detail_map_nm, 1 )
@end @property( !detail_maps_normal )
	@set( second_valid_detail_map_nm, 0 )
@end
