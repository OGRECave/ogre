@piece( DeclQuat_xAxis )
vec3 xAxis( vec4 qQuat )
{
	float fTy  = 2.0 * qQuat.y;
	float fTz  = 2.0 * qQuat.z;
	float fTwy = fTy * qQuat.w;
	float fTwz = fTz * qQuat.w;
	float fTxy = fTy * qQuat.x;
	float fTxz = fTz * qQuat.x;
	float fTyy = fTy * qQuat.y;
	float fTzz = fTz * qQuat.z;

	return vec3( 1.0-(fTyy+fTzz), fTxy+fTwz, fTxz-fTwy );
}
@end

@piece( DeclQuat_yAxis )
vec3 yAxis( vec4 qQuat )
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

	return vec3( fTxy-fTwz, 1.0-(fTxx+fTzz), fTyz+fTwx );
}
@end

@piece( DeclQuat_zAxis )
vec3 zAxis( vec4 qQuat )
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

	return vec3( fTxz+fTwy, fTyz-fTwx, 1.0-(fTxx+fTyy) );
}
@end

@piece( DeclQuat_AllAxis )
@insertpiece( DeclQuat_xAxis )
@insertpiece( DeclQuat_yAxis )
@insertpiece( DeclQuat_zAxis )
@end
