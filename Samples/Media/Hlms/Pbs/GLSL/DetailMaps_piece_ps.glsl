// detail_maps_diffuse & detail_maps_normal are either 0 or 4

@pmax( NumDetailMaps, detail_maps_diffuse, detail_maps_normal )
@foreach( NumDetailMaps, n )
	@property( detail_offsets@n )
		@piece( offsetDetail@n ) * material.detailOffsetScale[@n].zw + material.detailOffsetScale[@n].xy@end
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
