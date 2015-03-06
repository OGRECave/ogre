@piece( DeclQuat_xAxis )
float3 xAxis( float4 qQuat )
{
	float fTy  = 2.0 * qQuat.y;
	float fTz  = 2.0 * qQuat.z;
	float fTwy = fTy * qQuat.w;
	float fTwz = fTz * qQuat.w;
	float fTxy = fTy * qQuat.x;
	float fTxz = fTz * qQuat.x;
	float fTyy = fTy * qQuat.y;
	float fTzz = fTz * qQuat.z;

	return float3( 1.0-(fTyy+fTzz), fTxy+fTwz, fTxz-fTwy );
}
@end

@piece( DeclQuat_yAxis )
float3 yAxis( float4 qQuat )
{
	float fTx  = 2.0 * qQuat.x;
	float fTy  = 2.0 * qQuat.y;
	float fTz  = 2.0 * qQuat.z;
	float fTwx = fTx * qQuat.w;
	float fTwz = fTz * qQuat.w;
	float fTxx = fTx * qQuat.x;
	float fTxy = fTy * qQuat.x;
	float fTyz = fTz * qQuat.y;
	float fTzz = fTz * qQuat.z;

	return float3( fTxy-fTwz, 1.0-(fTxx+fTzz), fTyz+fTwx );
}
@end

@piece( DeclQuat_zAxis )
float3 zAxis( float4 qQuat )
{
	float fTx  = 2.0 * qQuat.x;
	float fTy  = 2.0 * qQuat.y;
	float fTz  = 2.0 * qQuat.z;
	float fTwx = fTx * qQuat.w;
	float fTwy = fTy * qQuat.w;
	float fTxx = fTx * qQuat.x;
	float fTxz = fTz * qQuat.x;
	float fTyy = fTy * qQuat.y;
	float fTyz = fTz * qQuat.y;

	return float3( fTxz+fTwy, fTyz-fTwx, 1.0-(fTxx+fTyy) );
}
@end

@piece( DeclQuat_AllAxis )
@insertpiece( DeclQuat_xAxis )
@insertpiece( DeclQuat_yAxis )
@insertpiece( DeclQuat_zAxis )
@end
