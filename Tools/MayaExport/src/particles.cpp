/****************************************************************
 Thanks to Bandures for the particle exporter
 ****************************************************************/

/*********************************************************************************
*                                                                                *
*   This program is free software; you can redistribute it and/or modify         *
*   it under the terms of the GNU Lesser General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or            *
*   (at your option) any later version.                                          *
*                                                                                *
**********************************************************************************/

#include "particles.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
namespace OgreMayaExporter
{
////////////////////////////////////////////////////////////////////////////////////////////////////
const float FP_2PI = 6.28318531f;
static float FP_MINDELTA = 0.005f;
////////////////////////////////////////////////////////////////////////////////////////////////////
inline float ToRadians( float fD ) { return fD * ( FP_2PI / 360.f ); }
inline float ToDegrees( float fR ) { return fR * ( 360.f / FP_2PI ); }
template<class T> void DropSame( const std::vector<T> &values, int nBeginTime, TKeyTrack<T> *pRes );
template<class T> void MakeLinearSpline( const std::vector<T> &values, int nBeginTime, float fEpsilon, TKeyTrack<T> *pRes );
template<class T> void writeTrackToXML( std::ofstream &outStream, const TKeyTrack<T> &dataTrack, const std::string &name );
////////////////////////////////////////////////////////////////////////////////////////////////////
// Particles
////////////////////////////////////////////////////////////////////////////////////////////////////
Particles::Particles():
    nFrames(0)
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
Particles::~Particles()
{
    clear();
}
////////////////////////////////////////////////////////////////////////////////////////////////////
MStatus Particles::load( MDagPath& dagPath, ParamList& params )
{
    int nMaxFrame, nMinFrame;
    MGlobal::executeCommand( "playbackOptions -q -max", nMaxFrame );
    MGlobal::executeCommand( "playbackOptions -q -min", nMinFrame );
    if ( nMinFrame > 0 )
        nMinFrame = 0;

    for ( int nFrame = nMinFrame; nFrame <= nMaxFrame; ++nFrame )
        ExportFrame( dagPath, nFrame );

    FinalizeData( nMinFrame, nMaxFrame );
    return MS::kSuccess;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
MStatus Particles::writeToXML( ParamList& params )
{
    params.outParticles << "<particles frames=\"" << nFrames << "\" tracks=\"" << particleTracks.size() << "\" >\n";
    for ( uint nTemp = 0; nTemp < particleTracks.size(); ++nTemp )
    {
        const SParticle &particle = particleTracks[nTemp];
        params.outParticles << "\t<particle startFrame=\"" << particle.nStartTime << "\" endFrame=\"" << particle.nEndTime << "\" >\n";
        writeTrackToXML( params.outParticles, particle.pos, "position" );
        writeTrackToXML( params.outParticles, particle.rotation, "rotation" );
        writeTrackToXML( params.outParticles, particle.scale, "scale" );
        writeTrackToXML( params.outParticles, particle.color, "color" );
        writeTrackToXML( params.outParticles, particle.sprite, "sprite" );
        params.outParticles << "\t</particle>\n";
    }
    params.outParticles << "</particles>\n";
    return MS::kSuccess;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void Particles::clear()
{
}
////////////////////////////////////////////////////////////////////////////////////////////////////
MStatus Particles::ExportFrame( MDagPath &dagPath, int nFrame )
{
    MGlobal::viewFrame( nFrame );

    MStatus retStatus;
    MFnDagNode dagNode( dagPath );
    ////
    int nParticles = 0;
    MPlug countPlug = dagNode.findPlug( MString( "count" ), &retStatus );
    retStatus = countPlug.getValue( nParticles );
    if ( nParticles <= 0 )
        return MS::kFailure;
    ////
    std::vector<int> idIndex( nParticles );
    std::vector<int> sortedId( nParticles );
    std::vector<SParticleData> particlesFrame( nParticles );
    ////
    MObject tempObj;
    MPlug mappingPlug = dagNode.findPlug( MString( "idMapping" ), &retStatus );
    ////
    MPlug idPlug = mappingPlug.child( 0 );
    retStatus = idPlug.getValue( tempObj );
    MFnIntArrayData sSortedIDArray( tempObj, &retStatus );
    for ( int nTemp = 0; nTemp < nParticles; ++nTemp )
        sortedId[nTemp] = sSortedIDArray[nTemp];
    ////
    MPlug indexPlug = mappingPlug.child( 1 );
    retStatus = indexPlug.getValue( tempObj );
    MFnIntArrayData idIndexArray( tempObj, &retStatus );
    for ( int nTemp = 0; nTemp < nParticles; ++nTemp )
        idIndex[nTemp] = idIndexArray[nTemp];
    //// Position
    MPlug posPlug = dagNode.findPlug( MString( "worldPosition" ), &retStatus );
    retStatus = posPlug.getValue( tempObj );
    MFnVectorArrayData posData( tempObj, &retStatus );
    for ( int nTemp = 0; nTemp < nParticles; ++nTemp )
    {
        SParticleData &particle = particlesFrame[nTemp];
        particle.nFrame = nFrame;
        particle.pos.x = (float)posData[idIndex[nTemp]].x;
        particle.pos.y = (float)posData[idIndex[nTemp]].y;
        particle.pos.z = (float)posData[idIndex[nTemp]].z;
    }
    //// Rotation
    MPlug rotPlug = dagNode.findPlug( MString( "spriteTwistPP" ), &retStatus );
    if ( retStatus == MS::kSuccess )
    {
        retStatus = rotPlug.getValue( tempObj );
        MFnDoubleArrayData rotPlug( tempObj, &retStatus );
        for ( int nTemp = 0; nTemp < nParticles; ++nTemp )
            particlesFrame[nTemp].fRotation = ToRadians( (float)rotPlug[idIndex[nTemp]] );
    }
    //// ScaleX
    MPlug scaleXPlug = dagNode.findPlug( MString( "spriteScaleXPP" ), &retStatus );
    if ( retStatus == MS::kSuccess )
    {
        retStatus = scaleXPlug.getValue( tempObj );
        MFnDoubleArrayData scaleX( tempObj, &retStatus );
        for ( int nTemp = 0; nTemp < nParticles; ++nTemp )
            particlesFrame[nTemp].scale.x = float( scaleX[idIndex[nTemp]] );
    }
    //// ScaleY
    MPlug scaleYPlug = dagNode.findPlug( MString( "spriteScaleYPP" ), &retStatus );
    if ( retStatus == MS::kSuccess )
    {
        retStatus = scaleYPlug.getValue( tempObj );
        MFnDoubleArrayData scaleY( tempObj, &retStatus );
        for ( int nTemp = 0; nTemp < nParticles; ++nTemp )
            particlesFrame[nTemp].scale.y = float( scaleY[idIndex[nTemp]] );
    }
    //// Sprite
    MPlug spritePlug = dagNode.findPlug( MString( "spriteNumPP" ), &retStatus );
    if ( retStatus == MS::kSuccess )
    {
        retStatus = spritePlug.getValue( tempObj );
        MFnDoubleArrayData sprite( tempObj, &retStatus );
        for ( int nTemp = 0; nTemp < nParticles; ++nTemp )
            particlesFrame[nTemp].nSprite = int( sprite[idIndex[nTemp]] - 1 );
    }
    //// Color
    MPlug colorPlug = dagNode.findPlug( MString( "rgbPP" ), &retStatus );
    if ( retStatus == MS::kSuccess )
    {
        retStatus = colorPlug.getValue( tempObj );
        MFnVectorArrayData rgbData( tempObj, &retStatus );
        for ( int nTemp = 0; nTemp < nParticles; ++nTemp )
        {
            particlesFrame[nTemp].color.r = float( rgbData[idIndex[nTemp]].x );
            particlesFrame[nTemp].color.g = float( rgbData[idIndex[nTemp]].y );
            particlesFrame[nTemp].color.b = float( rgbData[idIndex[nTemp]].z );
        }
    }
    //// Opacity
    MPlug alphaPlug = dagNode.findPlug( MString( "opacityPP" ), &retStatus );
    if ( retStatus == MS::kSuccess )
    {
        retStatus = alphaPlug.getValue( tempObj );
        MFnDoubleArrayData alphaData( tempObj, &retStatus );
        for ( int nTemp = 0; nTemp < nParticles; ++nTemp )
            particlesFrame[nTemp].color.a = float( alphaData[idIndex[nTemp]] );
    }

    for ( int nTemp = 0; nTemp < nParticles; ++nTemp )
        data[sortedId[nTemp]].push_back( particlesFrame[nTemp] );

    return MS::kSuccess;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
MStatus Particles::FinalizeData( int nMinFrame, int nMaxFrame )
{
    nFrames = nMaxFrame - nMinFrame + 1;
    particleTracks.resize( data.size() );

    int nTemp = 0;
    for ( CParticlesData::const_iterator iTemp = data.begin(); iTemp != data.end(); ++iTemp, ++nTemp )
    {
        SParticle &particle = particleTracks[nTemp];
        ////
        const CParticlesTrack &particlesTrack = iTemp->second;
        int nEndFrame = particlesTrack.back().nFrame;
        int nStartFrame = particlesTrack.front().nFrame;
        int nFrames = nEndFrame - nStartFrame + 1;
        ////
        if ( nFrames != particlesTrack.size() )
        {
            std::cout << "ERROR: particle dosn't exist in some frames (unsupported)!\n";
            std::cout.flush();
            return MS::kFailure;
        }
        ////
        int nBeginTime = nStartFrame;
        particle.nEndTime = nEndFrame;
        particle.nStartTime = nStartFrame;
        ////
        std::vector<SPos> tmpPos( nFrames );
        for ( int nTemp = 0; nTemp < nFrames; ++nTemp )
            tmpPos[nTemp] = particlesTrack[nTemp].pos;
        MakeLinearSpline( tmpPos, nBeginTime, FP_MINDELTA, &particle.pos );
        ////
        std::vector<float> tmpRot( nFrames );
        for ( int nTemp = 0; nTemp < nFrames; ++nTemp )
            tmpRot[nTemp] = particlesTrack[nTemp].fRotation;
        MakeLinearSpline( tmpRot, nBeginTime, FP_MINDELTA, &particle.rotation );
        ////
        std::vector<SScale> tmpScale( nFrames );
        for ( int nTemp = 0; nTemp < nFrames; ++nTemp )
            tmpScale[nTemp] = particlesTrack[nTemp].scale;
        MakeLinearSpline( tmpScale, nBeginTime, FP_MINDELTA, &particle.scale );
        ////
        std::vector<SColor> tmpColor( nFrames );
        for ( int nTemp = 0; nTemp < nFrames; ++nTemp )
            tmpColor[nTemp] = particlesTrack[nTemp].color;
        MakeLinearSpline( tmpColor, nBeginTime, FP_MINDELTA, &particle.color );
        ////
        std::vector<int> tmpSprite( nFrames );
        for ( int nTemp = 0; nTemp < nFrames; ++nTemp )
            tmpSprite[nTemp] = particlesTrack[nTemp].nSprite;
        DropSame( tmpSprite, nBeginTime, &particle.sprite );
    }

    return MS::kSuccess;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
bool CanDropValue( const T &value, const T &prevVal, const T &nextVal, int nDelta, int nRange, float fEpsilon )
{
    T resVal;
    float fCoeff = float( nDelta ) / nRange;
    Interpolate( prevVal, nextVal, fCoeff, &resVal );
    if ( fabs( resVal - value ) < fEpsilon )
        return true;
    ////
    return false;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
void MakeLinearSpline( const std::vector<T> &values, int nBeginTime, float fEpsilon, TKeyTrack<T> *pRes )
{
    if ( values.size() == 0 )
        return;
    ////
    TKey<T> startKey;
    startKey.nTime = nBeginTime;
    startKey.value = values.front();
    pRes->keys.push_back( startKey );
    if ( values.size() == 1 )
        return;
    ////
    uint nIndex = 0;
    uint nPrevIndex = 0;
    while( nIndex < values.size() - 1 )
    {
        if ( !CanDropValue<T>( values[nIndex], values[nPrevIndex], values[nIndex + 1], nIndex - nPrevIndex, nIndex - nPrevIndex + 1, fEpsilon ) )
        {
            nPrevIndex = nIndex;
            TKey<T> resKey;
            resKey.nTime = nBeginTime + nIndex;
            resKey.value = values[nIndex];
            pRes->keys.push_back( resKey );
        }
        ////
        nIndex++;
    }
    ////
    TKey<T> endKey;
    endKey.nTime = nBeginTime + values.size() - 1;
    endKey.value = values.back();
    pRes->keys.push_back( endKey );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
void DropSame( const std::vector<T> &values, int nBeginTime, TKeyTrack<T> *pRes )
{
    if ( values.size() == 0 )
        return;
    ////
    TKey<T> startKey;
    startKey.nTime = nBeginTime;
    startKey.value = values.front();
    pRes->keys.push_back( startKey );
    if ( values.size() == 1 )
        return;
    ////
    T nCurrent = values.front();
    TKey<T> curKey;
    for ( uint nTemp = 1; nTemp < values.size() - 1; ++nTemp )
    {
        if ( values[nTemp] != nCurrent )
        {
            curKey.nTime = nBeginTime + nTemp;
            curKey.value = values[nTemp];
            pRes->keys.push_back( curKey );
            ////
            nCurrent = values[nTemp];
        }
    }
    ////
    TKey<T> endKey;
    endKey.nTime = nBeginTime + values.size() - 1;
    endKey.value = values.back();
    pRes->keys.push_back( endKey );
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
void writeKeyToXML( std::ofstream &outStream, const TKey<T> &data )
{
    outStream << "\t\t\t<key time=\"" << data.nTime << "\" value=\"" << data.value << "\"/>\n";
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void writeKeyToXML( std::ofstream &outStream, const TKey<SPos> &data )
{
    outStream << "\t\t\t<key time=\"" << data.nTime << "\" x=\"" << data.value.x << "\" y=\"" << data.value.y << "\" z=\"" << data.value.z << "\"/>\n";
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void writeKeyToXML( std::ofstream &outStream, const TKey<SColor> &data )
{
    outStream << "\t\t\t<key time=\"" << data.nTime << "\" r=\"" << data.value.x << "\" g=\"" << data.value.y << "\" b=\"" << data.value.z << "\" a=\"" << data.value.a << "\"/>\n";
}
////////////////////////////////////////////////////////////////////////////////////////////////////
void writeKeyToXML( std::ofstream &outStream, const TKey<SScale> &data )
{
    outStream << "\t\t\t<key time=\"" << data.nTime << "\" x=\"" << data.value.x << "\" y=\"" << data.value.y << "\"/>\n";
}
////////////////////////////////////////////////////////////////////////////////////////////////////
template<class T>
void writeTrackToXML( std::ofstream &outStream, const TKeyTrack<T> &dataTrack, const std::string &name )
{
    outStream << "\t\t<" << name.c_str() << " keys=\"" << dataTrack.keys.size() << "\" >\n";
    for ( uint nTemp = 0; nTemp < dataTrack.keys.size(); ++nTemp )
        writeKeyToXML( outStream, dataTrack.keys[nTemp] );
    outStream << "\t\t</" << name.c_str() << ">\n";
}
////////////////////////////////////////////////////////////////////////////////////////////////////
} // NAMESPACE
////////////////////////////////////////////////////////////////////////////////////////////////////