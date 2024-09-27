# Particle Scripts {#Particle-Scripts}

Particle scripts allow you to define particle systems to be instantiated in your code without having to hard-code the settings themselves in your source code, allowing a very quick turnaround on any changes you make. Particle systems which are defined in scripts are used as templates, and multiple actual systems can be created from them at runtime.

@tableofcontents

Once scripts have been parsed, your code is free to instantiate systems based on them using the Ogre::SceneManager::createParticleSystem() method which can take both a name for the new system, and the name of the template to base it on (this template name is in the script).

@snippet Samples/Media/particle/Examples.particle manual_sample

A system can have top-level attributes set using the scripting commands available, such as ’quota’ to set the maximum number of particles allowed in the system. Emitters (which create particles) and affectors (which modify particles) are added as nested definitions within the script. The parameters available in the emitter and affector sections are entirely dependent on the type of emitter / affector.

For a detailed description of the core particle system attributes, see the list below:

## Particle System Attributes {#Particle-System-Attributes}

This section describes to attributes which you can set on every particle system using scripts. All attributes have default values so all settings are optional in your script.

-   [quota](#quota)
-   [material](#particle_005fmaterial)
-   [particle_width](#particle_005fwidth)
-   [particle_height](#particle_005fheight)
-   [cull_each](#cull_005feach)
-   [renderer](#particle_005frenderer)
-   [sorted](#particle_005fsorted)
-   [local_space](#particle_005flocalspace)
-   [iteration_interval](#iteration_005finterval)
-   [nonvisible_update_timeout](#nonvisible_005fupdate_005ftimeout)

@par Billboard Renderer Attributes

-   [billboard_type](#billboard_005ftype)
-   [billboard_origin](#billboard_005forigin)
-   [billboard_rotation_type](#billboard_005frotation_005ftype)
-   [common_direction](#common_005fdirection)
-   [common_up_vector](#common_005fup_005fvector)
-   [point_rendering](#particle_005fpoint_005frendering)
-   [accurate_facing](#particle_005faccurate_005ffacing)

@see @ref Particle-Emitters
@see @ref Particle-Affectors


<a name="quota"></a><a name="quota-1"></a>

### quota

Sets the maximum number of particles this system is allowed to contain at one time. When this limit is exhausted, the emitters will not be allowed to emit any more particles until some destroyed (e.g. through their time\_to\_live running out). Note that you will almost always want to change this, since it defaults to a very low value (particle pools are only ever increased in size, never decreased).

format: quota &lt;max\_particles&gt;<br> example: quota 10000<br> default: 10<br>

<a name="particle_005fmaterial"></a><a name="material-2"></a>

### material

Sets the name of the material which all particles in this system will use. All particles in a system use the same material, although each particle can tint this material through the use of its colour property.

format: material &lt;material\_name&gt;<br> example: material Examples/Flare<br> default: none (blank material)<br>

<a name="particle_005fwidth"></a><a name="particle_005fwidth-1"></a>

### particle\_width

Sets the width of particles in world coordinates. Note that this property is absolute when billboard\_type (see below) is set to ’point’ or ’perpendicular\_self’, but is scaled by the length of the direction vector when billboard\_type is ’oriented\_common’, ’oriented\_self’ or ’perpendicular\_common’.<br>

format: particle\_width &lt;width&gt;<br> example: particle\_width 20<br> default: 100<br>

<a name="particle_005fheight"></a><a name="particle_005fheight-1"></a>

### particle\_height

Sets the height of particles in world coordinates. Note that this property is absolute when billboard\_type (see below) is set to ’point’ or ’perpendicular\_self’, but is scaled by the length of the direction vector when billboard\_type is ’oriented\_common’, ’oriented\_self’ or ’perpendicular\_common’.<br>

format: particle\_height &lt;height&gt;<br> example: particle\_height 20<br> default: 100<br>

<a name="cull_005feach"></a><a name="cull_005feach-1"></a>

### cull\_each

All particle systems are culled by the bounding box which contains all the particles in the system. This is normally sufficient for fairly locally constrained particle systems where most particles are either visible or not visible together. However, for those that spread particles over a wider area (e.g. a rain system), you may want to actually cull each particle individually to save on time, since it is far more likely that only a subset of the particles will be visible. You do this by setting the cull\_each parameter to true.

format: cull\_each &lt;true|false&gt;<br> example: cull\_each true<br> default: false<br>

<a name="particle_005frenderer"></a><a name="renderer"></a>

### renderer

Particle systems do not render themselves, they do it through ParticleRenderer classes. Those classes are registered with a manager in order to provide particle systems with a particular ’look’. OGRE comes configured with a default billboard-based renderer, but more can be added through plugins. Particle renders are registered with a unique name, and you can use that name in this attribute to determine the renderer to use. The default is ’billboard’.

Particle renderers can have attributes, which can be passed by setting them on the root particle system.

format: renderer &lt;renderer\_name&gt;<br> default: billboard<br>

<a name="particle_005fsorted"></a><a name="sorted"></a>

### sorted

By default, particles are not sorted. By setting this attribute to ’true’, the particles will be sorted with respect to the camera, furthest first. This can make certain rendering effects look better at a small sorting expense.

format: sorted &lt;true|false&gt;<br> default: false<br>

<a name="particle_005flocalspace"></a><a name="local_005fspace"></a>

### local\_space

By default, particles are emitted into world space, such that if you transform the node to which the system is attached, it will not affect the particles (only the emitters). This tends to give the normal expected behaviour, which is to model how real world particles travel independently from the objects they are emitted from. However, to create some effects you may want the particles to remain attached to the local space the emitter is in and to follow them directly. This option allows you to do that.

format: local\_space &lt;true|false&gt;<br> default: false<br>

<a name="iteration_005finterval"></a><a name="iteration_005finterval-1"></a>

### iteration\_interval

Usually particle systems are updated based on the frame rate; however this can give variable results with more extreme frame rate ranges, particularly at lower frame rates. You can use this option to make the update frequency a fixed interval, whereby at lower frame rates, the particle update will be repeated at the fixed interval until the frame time is used up. A value of 0 means the default frame time iteration.

format: iteration\_interval &lt;secs&gt;<br> example: iteration\_interval 0.01<br> default: iteration\_interval 0<br>

<a name="nonvisible_005fupdate_005ftimeout"></a><a name="nonvisible_005fupdate_005ftimeout-1"></a>

### nonvisible\_update\_timeout

Sets when the particle system should stop updating after it hasn’t been visible for a while. By default, visible particle systems update all the time, even when not in view. This means that they are guaranteed to be consistent when they do enter view. However, this comes at a cost, updating particle systems can be expensive, especially if they are perpetual.  This option lets you set a ’timeout’ on the particle system, so that if it isn’t visible for this amount of time, it will stop updating until it is next visible. A value of 0 disables the timeout and always updates.

format: nonvisible\_update\_timeout &lt;secs&gt;<br> example: nonvisible\_update\_timeout 10<br> default: nonvisible\_update\_timeout 0<br>

## Billboard Renderer Attributes {#Billboard-Renderer-Attributes}

These are actually attributes of the @c billboard particle renderer (the default), but can be passed to a particle renderer by declaring them directly within the system declaration. Particles using the default renderer are rendered using billboards, which are rectangles formed by 2 triangles which rotate to face the given direction.

<a name="billboard_005ftype"></a><a name="billboard_005ftype-1"></a>

### billboard\_type

There is more than 1 way to orient a billboard. The classic approach is for the billboard to directly face the camera: this is the default behaviour. However this arrangement only looks good for particles which are representing something vaguely spherical like a light flare. For more linear effects like laser fire, you actually want the particle to have an orientation of its own.

format: billboard\_type &lt;point|oriented\_common|oriented\_self|perpendicular\_common|perpendicular\_self&gt;<br> example: billboard\_type oriented\_self<br> default: point<br>

The options for this parameter are:

<dl compact="compact">
<dt>point</dt> <dd>

The default arrangement, this approximates spherical particles and the billboards always fully face the camera.

</dd> <dt>oriented\_common</dt> <dd>

Particles are oriented around a common, typically fixed direction vector (see [common_direction](#common_005fdirection)), which acts as their local Y axis. The billboard rotates only around this axis, giving the particle some sense of direction. Good for rainstorms, starfields etc where the particles will traveling in one direction - this is slightly faster than oriented\_self (see below).

</dd> <dt>oriented\_self</dt> <dd>

Particles are oriented around their own direction vector, which acts as their local Y axis. As the particle changes direction, so the billboard reorients itself to face this way. Good for laser fire, fireworks and other ’streaky’ particles that should look like they are traveling in their own direction.

</dd> <dt>perpendicular\_common</dt> <dd>

Particles are perpendicular to a common, typically fixed direction vector (see [common_direction](#common_005fdirection)), which acts as their local Z axis, and their local Y axis coplanar with common direction and the common up vector (see [common_up_vector](#common_005fup_005fvector)). The billboard never rotates to face the camera, you might use double-side material to ensure particles never culled by back-facing. Good for aureolas, rings etc where the particles will perpendicular to the ground - this is slightly faster than perpendicular\_self (see below).

</dd> <dt>perpendicular\_self</dt> <dd>

Particles are perpendicular to their own direction vector, which acts as their local Z axis, and their local Y axis coplanar with their own direction vector and the common up vector (see [common_up_vector](#common_005fup_005fvector)). The billboard never rotates to face the camera, you might use double-side material to ensure particles never culled by back-facing. Good for rings stack etc where the particles will perpendicular to their traveling direction.

</dd> </dl> <a name="billboard_005forigin"></a><a name="billboard_005forigin-1"></a>

### billboard\_origin

@copydetails Ogre::BillboardOrigin

format: billboard\_origin &lt;top\_left|top\_center|top\_right|center\_left|center|center\_right|bottom\_left|bottom\_center|bottom\_right&gt;<br> example: billboard\_origin top\_right<br> default: center<br>

<a name="billboard_005frotation_005ftype"></a><a name="billboard_005frotation_005ftype-1"></a>

### billboard\_rotation\_type

@copydetails Ogre::BillboardRotationType

format: billboard\_rotation\_type &lt;vertex|texcoord&gt;<br> example: billboard\_rotation\_type vertex<br> default: texcoord<br>

The options for this parameter are:

<dl compact="compact">
<dt>vertex</dt> <dd>

Billboard particles will rotate the vertices around their facing direction to according with particle rotation. Rotate vertices guarantee texture corners exactly match billboard corners, thus has advantage mentioned above, but should take more time to generate the vertices.

</dd> <dt>texcoord</dt> <dd>

Billboard particles will rotate the texture coordinates to according with particle rotation. Rotate texture coordinates is faster than rotate vertices, but has some disadvantage mentioned above.

</dd> </dl>

<a name="common_005fdirection"></a><a name="common_005fdirection-1"></a>

### common\_direction

Only required if [billboard_type](#billboard_005ftype) is set to oriented\_common or perpendicular\_common, this vector is the common direction vector used to orient all particles in the system.

format: common\_direction &lt;x&gt; &lt;y&gt; &lt;z&gt;<br> example: common\_direction 0 -1 0<br> default: 0 0 1<br>

<a name="common_005fup_005fvector"></a><a name="common_005fup_005fvector-1"></a>

### common\_up\_vector

Only required if [billboard_type](#billboard_005ftype) is set to perpendicular\_self or perpendicular\_common, this vector is the common up vector used to orient all particles in the system.

format: common\_up\_vector &lt;x&gt; &lt;y&gt; &lt;z&gt;<br> example: common\_up\_vector 0 1 0<br> default: 0 1 0<br>

<a name="particle_005fpoint_005frendering"></a><a name="point_005frendering"></a>

### point\_rendering

This sets whether or not the BillboardSet will use point rendering rather than manually generated quads.

@copydetails Ogre::BillboardSet::setPointRenderingEnabled

<a name="particle_005faccurate_005ffacing"></a><a name="accurate_005ffacing"></a>

### accurate\_facing

This sets whether or not the BillboardSet will use a slower but more accurate calculation for facing the billboard to the camera. By default it uses the camera direction, which is faster but means the billboards don’t stay in the same orientation as you rotate the camera. The ’accurate\_facing on’ option makes the calculation based on a vector from each billboard to the camera, which means the orientation is constant even whilst the camera rotates.

format: accurate\_facing on|off<br> default: accurate\_facing off<br>

### texture_sheet_size

format: texture_sheet_size &lt;stacks&gt; &lt;slices&gt;

@copydetails Ogre::BillboardSet::setTextureStacksAndSlices

# Particle Emitters {#Particle-Emitters}

Particle emitters are classified by ’type’ e.g. ’Point’ emitters emit from a single point whilst ’Box’ emitters emit randomly from an area. New emitters can be added to Ogre by creating plugins. You add an emitter to a system by nesting another section within it, headed with the keyword ’emitter’ followed by the name of the type of emitter (case sensitive). Ogre currently supports ’Point’, ’Box’, ’Cylinder’, ’Ellipsoid’, ’HollowEllipsoid’ and ’Ring’ emitters.

@see @ref Particle-Affectors

## Emitting Emitters {#Emitting-Emitters}

It is possible to ’emit emitters’ - that is, have new emitters spawned based on the position of particles,, for example to product ’firework’ style effects.

This is controlled via the following directives:

<dl compact="compact">
<dt>emit\_emitter\_quota</dt> <dd>

This parameter is a system-level parameter telling the system how many emitted emitters may be in use at any one time. This is just to allow for the space allocation process.

</dd> <dt>name</dt> <dd>

This parameter is an emitter-level parameter, giving a name to an emitter. This can then be referred to in another emitter as the new emitter type to spawn.

</dd> <dt>emit\_emitter</dt> <dd>

This is an emitter-level parameter, and if specified, it means that the particles spawned by this emitter, are themselves emitters of the named type.

</dd> </dl>

<a name="Particle-Emitter-Attributes"></a> <a name="Particle-Emitter-Attributes-1"></a>

## Common Emitter Attributes

This section describes the common attributes of all particle emitters. Specific emitter types may also support their own extra attributes.

-   [angle](#angle)
-   [colour](#colour)
-   [colour_range_start](#colour_005frange_005fstart)
-   [colour_range_end](#colour_005frange_005fend)
-   [direction](#direction)
-   [direction_position_reference](#direction_005fposition_005freference)
-   [emission_rate](#emission_005frate)
-   [position](#position)
-   [velocity](#velocity)
-   [velocity_min](#velocity_005fmin)
-   [velocity_max](#velocity_005fmax)
-   [time_to_live](#time_005fto_005flive)
-   [time_to_live_min](#time_005fto_005flive_005fmin)
-   [time_to_live_max](#time_005fto_005flive_005fmax)
-   [duration](#duration)
-   [duration_min](#duration_005fmin)
-   [duration_max](#duration_005fmax)
-   [repeat_delay](#repeat_005fdelay)
-   [repeat_delay_min](#repeat_005fdelay_005fmin)
-   [repeat_delay_max](#repeat_005fdelay_005fmax)

<a name="angle"></a><a name="angle-1"></a>

## angle

Sets the maximum angle (in degrees) which emitted particles may deviate from the direction of the emitter (see direction). Setting this to 10 allows particles to deviate up to 10 degrees in any direction away from the emitter’s direction. A value of 180 means emit in any direction, whilst 0 means emit always exactly in the direction of the emitter.

format: angle &lt;degrees&gt;<br> example: angle 30<br> default: 0<br>

<a name="colour"></a><a name="colour-1"></a>

## colour

Sets a static colour for all particle emitted. Also see the colour\_range\_start and colour\_range\_end attributes for setting a range of colours. The format of the colour parameter is "r g b a", where each component is a value from 0 to 1, and the alpha value is optional (assumes 1 if not specified).

format: colour &lt;r&gt; &lt;g&gt; &lt;b&gt; \[&lt;a&gt;\]<br> example: colour 1 0 0 1<br> default: 1 1 1 1<br>

<a name="colour_005frange_005fstart"></a><a name="colour_005frange_005fend"></a>

## colour\_range\_start & colour\_range\_end

As the ’colour’ attribute, except these 2 attributes must be specified together, and indicate the range of colours available to emitted particles. The actual colour will be randomly chosen between these 2 values.

format: as colour<br> example (generates random colours between red and blue):<br>     colour\_range\_start 1 0 0<br>     colour\_range\_end 0 0 1<br> default: both 1 1 1 1<br>

<a name="direction"></a><a name="direction-1"></a>

## direction

Sets the direction of the emitter. This is relative to the SceneNode which the particle system is attached to, meaning that as with other movable objects changing the orientation of the node will also move the emitter.

format: direction &lt;x&gt; &lt;y&gt; &lt;z&gt;<br> example: direction 0 1 0<br> default: 1 0 0<br>

<a name="direction_005fposition_005freference"></a><a name="direction_005fposition_005freference-1"></a>

## direction\_position\_reference

Sets the position reference of the emitter. This supersedes direction when present. The last parameter must be 1 to enable it, 0 to disable. You may still want to set the direction to setup orientation of the emitter’s dimensions. When present, particles direction is calculated at the time of emission by doing (particlePosition - referencePosition); therefore particles will travel in a particular direction or in every direction depending on where the particles are originated, and the location of the reference position. Note angle still works to apply some randomness after the direction vector is generated. This parameter is specially useful to create explosions and implosions (when velocity is negative) best paired with HollowEllipsoid and Ring emitters. This is relative to the SceneNode which the particle system is attached to, meaning that as with other movable objects changing the orientation of the node will also move the emitter.

format: direction\_position\_reference &lt;x&gt; &lt;y&gt; &lt;z&gt; &lt;enable&gt;<br> example: direction\_position\_reference 0 -10 0 1<br> default: direction\_position\_reference 0 0 0 0<br>

<a name="emission_005frate"></a><a name="emission_005frate-1"></a>

## emission\_rate

Sets how many particles per second should be emitted. The specific emitter does not have to emit these in a continuous manner - this is a relative parameter and the emitter may choose to emit all of the second’s worth of particles every half-second for example, the behaviour depends on the emitter. The emission rate will also be limited by the particle system’s ’quota’ setting.

format: emission\_rate &lt;particles\_per\_second&gt;<br> example: emission\_rate 50<br> default: 10<br>

<a name="position"></a><a name="position-1"></a>

## position

Sets the position of the emitter relative to the SceneNode the particle system is attached to.

format: position &lt;x&gt; &lt;y&gt; &lt;z&gt;<br> example: position 10 0 40<br> default: 0 0 0<br>

<a name="velocity"></a><a name="velocity-1"></a>

## velocity

Sets a constant velocity for all particles at emission time. See also the velocity\_min and velocity\_max attributes which allow you to set a range of velocities instead of a fixed one.

format: velocity &lt;world\_units\_per\_second&gt;<br> example: velocity 100<br> default: 1<br>

<a name="velocity_005fmin"></a><a name="velocity_005fmax"></a>

## velocity\_min & velocity\_max

As ’velocity’ except these attributes set a velocity range and each particle is emitted with a random velocity within this range.

format: as velocity<br> example:<br>     velocity\_min 50<br>     velocity\_max 100<br> default: both 1<br>

<a name="time_005fto_005flive"></a><a name="time_005fto_005flive-1"></a>

## time\_to\_live

Sets the number of seconds each particle will ’live’ for before being destroyed. NB it is possible for particle affectors to alter this in flight, but this is the value given to particles on emission. See also the time\_to\_live\_min and time\_to\_live\_max attributes which let you set a lifetime range instead of a fixed one.

format: time\_to\_live &lt;seconds&gt;<br> example: time\_to\_live 10<br> default: 5<br>

<a name="time_005fto_005flive_005fmin"></a><a name="time_005fto_005flive_005fmax"></a>

## time\_to\_live\_min & time\_to\_live\_max

As time\_to\_live, except this sets a range of lifetimes and each particle gets a random value in-between on emission.

format: as time\_to\_live<br> example:<br>     time\_to\_live\_min 2<br>     time\_to\_live\_max 5<br> default: both 5<br> <br>

<a name="duration"></a><a name="duration-1"></a>

## duration

Sets the number of seconds the emitter is active. The emitter can be started again, see [repeat_delay](#repeat_005fdelay).
See also the duration\_min and duration\_max attributes which let you set a duration range instead of a fixed one.

format: duration &lt;seconds&gt;<br> example:<br>     duration 2.5<br> default: 0<br> <br>

@note A value of 0 means infinite duration. A value < 0 means "burst" where @c emission_rate of particles are emitted once in the next frame.

<a name="duration_005fmin"></a><a name="duration_005fmax"></a>

## duration\_min & duration\_max

As duration, except these attributes set a variable time range between the min and max values each time the emitter is started.

format: as duration<br> example:<br>     duration\_min 2<br>     duration\_max 5<br> default: both 0<br> <br>

<a name="repeat_005fdelay"></a><a name="repeat_005fdelay-1"></a>

## repeat\_delay

Sets the number of seconds to wait before the emission is repeated when stopped by a limited [duration](#duration). See also the repeat\_delay\_min and repeat\_delay\_max attributes which allow you to set a range of repeat\_delays instead of a fixed one.

format: repeat\_delay &lt;seconds&gt;<br> example:<br>     repeat\_delay 2.5<br> default: 0<br> <br>

<a name="repeat_005fdelay_005fmin"></a><a name="repeat_005fdelay_005fmax"></a>

## repeat\_delay\_min & repeat\_delay\_max

As repeat\_delay, except this sets a range of repeat delays and each time the emitter is started it gets a random value in-between.

format: as repeat\_delay<br> example:<br>     repeat\_delay 2<br>     repeat\_delay 5<br> default: both 0<br> <br>

# Standard Particle Emitters {#Standard-Particle-Emitters}

Ogre comes preconfigured with a few particle emitters. New ones can be added by creating plugins: see the Plugin\_ParticleFX project as an example of how you would do this (this is where these emitters are implemented).

-   [Point Emitter](#Point-Emitter)
-   [Box Emitter](#Box-Emitter)
-   [Cylinder Emitter](#Cylinder-Emitter)
-   [Ellipsoid Emitter](#Ellipsoid-Emitter)
-   [Hollow Ellipsoid Emitter](#Hollow-Ellipsoid-Emitter)
-   [Ring Emitter](#Ring-Emitter)

## Point Emitter {#Point-Emitter}

@copybrief Ogre::PointEmitter

This emitter has no additional attributes over an above the standard emitter attributes.

To create a point emitter, include a section like this within your particle system script:

```cpp

emitter Point
{
    // Settings go here
}
```

## Box Emitter {#Box-Emitter}

@copybrief Ogre::BoxEmitter

Its extra attributes are:

<dl compact="compact">
<dt>width</dt> <dd>

Sets the width of the box (this is the size of the box along its local X axis, which is dependent on the ’direction’ attribute which forms the box’s local Z).<br> format: width &lt;units&gt;<br> example: width 250<br> default: 100<br>

</dd> <dt>height</dt> <dd>

Sets the height of the box (this is the size of the box along its local Y axis, which is dependent on the ’direction’ attribute which forms the box’s local Z).<br> format: height &lt;units&gt;<br> example: height 250<br> default: 100<br>

</dd> <dt>depth</dt> <dd>

Sets the depth of the box (this is the size of the box along its local Z axis, which is the same as the ’direction’ attribute).<br> format: depth &lt;units&gt;<br> example: depth 250<br> default: 100<br>

</dd> </dl> <br>

To create a box emitter, include a section like this within your particle system script:

```cpp
emitter Box
{
    // Settings go here
}
```

## Cylinder Emitter {#Cylinder-Emitter}

@copybrief Ogre::CylinderEmitter

This emitter has exactly the same parameters as the [Box Emitter](#Box-Emitter) so there are no additional parameters to consider here - the width and height determine the shape of the cylinder along its axis (if they are different it is an ellipsoid cylinder), the depth determines the length of the cylinder.

## Ellipsoid Emitter {#Ellipsoid-Emitter}

This emitter emits particles from within an ellipsoid shaped area, i.e. a sphere or squashed-sphere area. The parameters are again identical to the [Box Emitter](#Box-Emitter), except that the dimensions describe the widest points along each of the axes.

## Hollow Ellipsoid Emitter {#Hollow-Ellipsoid-Emitter}

This emitter is just like [Ellipsoid Emitter](#Ellipsoid-Emitter) except that there is a hollow area in the center of the ellipsoid from which no particles are emitted.

Therefore it has 3 extra parameters in order to define this area:

<dl compact="compact">
<dt>inner\_width</dt> <dd>

The width of the inner area which does not emit any particles.

</dd> <dt>inner\_height</dt> <dd>

The height of the inner area which does not emit any particles.

</dd> <dt>inner\_depth</dt> <dd>

The depth of the inner area which does not emit any particles.

</dd> </dl>

## Ring Emitter {#Ring-Emitter}

This emitter emits particles from a ring-shaped area, i.e. a little like [Hollow Ellipsoid Emitter](#Hollow-Ellipsoid-Emitter) except only in 2 dimensions.

<dl compact="compact">
<dt>inner\_width</dt> <dd>

The width of the inner area which does not emit any particles.

</dd> <dt>inner\_height</dt> <dd>

The height of the inner area which does not emit any particles.

</dd> </dl>

# Particle Affectors {#Particle-Affectors}

Particle affectors modify particles over their lifetime. They are classified by ’type’ e.g. ’LinearForce’ affectors apply a force to all particles, whilst ’ColourFader’ affectors alter the colour of particles in flight. New affectors can be added to Ogre by creating plugins. You add an affector to a system by nesting another section within it, headed with the keyword ’affector’ followed by the name of the type of affector (case sensitive).

Particle affectors actually have no universal attributes; they are all specific to the type of affector.

@see @ref Particle-Emitters

# Standard Particle Affectors {#Standard-Particle-Affectors}

Ogre comes preconfigured with a few particle affectors. New ones can be added by creating plugins: see the Plugin\_ParticleFX project as an example of how you would do this (this is where these affectors are implemented).

-   [Linear Force Affector](#Linear-Force-Affector)
-   [ColourFader Affector](#ColourFader-Affector)
-   [ColourFader2 Affector](#ColourFader2-Affector)
-   [Scaler Affector](#Scaler-Affector)
-   [Rotator Affector](#Rotator-Affector)
-   [ColourInterpolator Affector](#ColourInterpolator-Affector)
-   [ColourImage Affector](#ColourImage-Affector)
-   [DeflectorPlane Affector](#DeflectorPlane-Affector)
-   [DirectionRandomiser Affector](#DirectionRandomiser-Affector)
-   [TextureAnimator Affector](#TextureAnimator-Affector)

## Linear Force Affector {#Linear-Force-Affector}

@copybrief Ogre::LinearForceAffector

Its extra attributes are:

<dl compact="compact">
<dt>force\_vector</dt> <dd>

Sets the vector for the force to be applied to every particle. The magnitude of this vector determines how strong the force is.<br>     format: force\_vector &lt;x&gt; &lt;y&gt; &lt;z&gt;<br>     example: force\_vector 50 0 -50<br>     default: 0 -100 0 (a fair gravity effect)<br>

</dd> <dt>force\_application</dt> <dd>

Sets the way in which the force vector is applied to particle momentum.<br>     format: force\_application &lt;add|average&gt;<br>     example: force\_application average<br>     default: add<br> The options are:

<dl compact="compact">
<dt>average</dt> <dd>

The resulting momentum is the average of the force vector and the particle’s current motion. Is self-stabilising but the speed at which the particle changes direction is non-linear.

</dd> <dt>add</dt> <dd>

The resulting momentum is the particle’s current motion plus the force vector. This is traditional force acceleration but can potentially result in unlimited velocity.

</dd> </dl> </dd> </dl> <br>

To create a linear force affector, include a section like this within your particle system script:

```cpp
affector LinearForce
{
    // Settings go here
}
```

## ColourFader Affector {#ColourFader-Affector}

@copybrief Ogre::ColourFaderAffector

Its extra attributes are:

<dl compact="compact">
<dt>red</dt> <dd>

Sets the adjustment to be made to the red component of the particle colour per second.<br>     format: red &lt;delta\_value&gt;<br>     example: red -0.1<br>     default: 0<br>

</dd> <dt>green</dt> <dd>

Sets the adjustment to be made to the green component of the particle colour per second.<br>     format: green &lt;delta\_value&gt;<br>     example: green -0.1<br>     default: 0<br>

</dd> <dt>blue</dt> <dd>

Sets the adjustment to be made to the blue component of the particle colour per second.<br>     format: blue &lt;delta\_value&gt;<br>     example: blue -0.1<br>     default: 0<br>

</dd> <dt>alpha</dt> <dd>

Sets the adjustment to be made to the alpha component of the particle colour per second.<br>     format: alpha &lt;delta\_value&gt;<br> example: alpha -0.1<br> default: 0<br>

</dd> </dl>

To create a colour fader affector, include a section like this within your particle system script:

```cpp
affector ColourFader
{
    // Settings go here
}
```

## ColourFader2 Affector {#ColourFader2-Affector}

This affector is similar to the [ColourFader Affector](#ColourFader-Affector), except it introduces two states of colour changes as opposed to just one. The second colour change state is activated once a specified amount of time remains in the particles life.

<dl compact="compact">
<dt>red1</dt> <dd>

Sets the adjustment to be made to the red component of the particle colour per second for the first state.<br>     format: red &lt;delta\_value&gt;<br>     example: red -0.1<br>     default: 0<br>

</dd> <dt>green1</dt> <dd>

Sets the adjustment to be made to the green component of the particle colour per second for the first state.<br>     format: green &lt;delta\_value&gt;<br>     example: green -0.1<br>     default: 0<br>

</dd> <dt>blue1</dt> <dd>

Sets the adjustment to be made to the blue component of the particle colour per second for the first state.<br>     format: blue &lt;delta\_value&gt;<br>     example: blue -0.1<br>     default: 0<br>

</dd> <dt>alpha1</dt> <dd>

Sets the adjustment to be made to the alpha component of the particle colour per second for the first state.<br>     format: alpha &lt;delta\_value&gt;<br> example: alpha -0.1<br> default: 0<br>

</dd> <dt>red2</dt> <dd>

Sets the adjustment to be made to the red component of the particle colour per second for the second state.<br>     format: red &lt;delta\_value&gt;<br>     example: red -0.1<br>     default: 0<br>

</dd> <dt>green2</dt> <dd>

Sets the adjustment to be made to the green component of the particle colour per second for the second state.<br>     format: green &lt;delta\_value&gt;<br>     example: green -0.1<br>     default: 0<br>

</dd> <dt>blue2</dt> <dd>

Sets the adjustment to be made to the blue component of the particle colour per second for the second state.<br>     format: blue &lt;delta\_value&gt;<br>     example: blue -0.1<br>     default: 0<br>

</dd> <dt>alpha2</dt> <dd>

Sets the adjustment to be made to the alpha component of the particle colour per second for the second state.<br>     format: alpha &lt;delta\_value&gt;<br> example: alpha -0.1<br> default: 0<br>

</dd> <dt>state\_change</dt> <dd>

When a particle has this much time left to live, it will switch to state 2.<br>     format: state\_change &lt;seconds&gt;<br> example: state\_change 2<br> default: 1<br>

</dd> </dl>

To create a ColourFader2 affector, include a section like this within your particle system script:

```cpp
affector ColourFader2
{
    // Settings go here
}
```

## Scaler Affector {#Scaler-Affector}

@copybrief Ogre::ScaleAffector

Its extra attributes are:


@par rate
The amount by which to scale the particles in both the x and y direction per second.

@par scale_range
The scale factor range to be applied to emitted particles.
@par
example: scale_range 0.5 1.5<br>
default: 1.0 1.0<br>

To create a scale affector, include a section like this within your particle system script:

```cpp
affector Scaler
{
    // Settings go here
}
```

## Rotator Affector {#Rotator-Affector}

@copybrief Ogre::RotationAffector

Its extra attributes are:

<dl compact="compact">
<dt>rotation\_speed\_range\_start</dt> <dd>

The start of a range of rotation speeds to be assigned to emitted particles.<br>     format: rotation\_speed\_range\_start &lt;degrees\_per\_second&gt;<br> example: rotation\_speed\_range\_start 90<br> default: 0<br>

</dd> <dt>rotation\_speed\_range\_end</dt> <dd>

The end of a range of rotation speeds to be assigned to emitted particles.<br>     format: rotation\_speed\_range\_end &lt;degrees\_per\_second&gt;<br> example: rotation\_speed\_range\_end 180<br> default: 0<br>

</dd> <dt>rotation\_range\_start</dt> <dd>

The start of a range of rotation angles to be assigned to emitted particles.<br>     format: rotation\_range\_start &lt;degrees&gt;<br> example: rotation\_range\_start 0<br> default: 0<br>

</dd> <dt>rotation\_range\_end</dt> <dd>

The end of a range of rotation angles to be assigned to emitted particles.<br>     format: rotation\_range\_end &lt;degrees&gt;<br> example: rotation\_range\_end 360<br> default: 0<br>

</dd> </dl>

To create a rotate affector, include a section like this within your particle system script:

```cpp
affector Rotator
{
    // Settings go here
}
```

## ColourInterpolator Affector {#ColourInterpolator-Affector}

Similar to the ColourFader and ColourFader2 Affectors, this affector modifies the colour of particles in flight, except it has a variable number of defined stages. It swaps the particle colour for several stages in the life of a particle and interpolates between them.

Its extra attributes are:

<dl compact="compact">
<dt>time0</dt> <dd>

The point in time of stage 0.<br>     format: time0 &lt;0-1 based on lifetime&gt;<br> example: time0 0<br> default: 1<br>

</dd> <dt>colour0</dt> <dd>

The colour at stage 0.<br>     format: colour0 &lt;r&gt; &lt;g&gt; &lt;b&gt; \[&lt;a&gt;\]<br> example: colour0 1 0 0 1<br> default: 0.5 0.5 0.5 0.0<br>

</dd> <dt>time1</dt> <dd>

The point in time of stage 1.<br>     format: time1 &lt;0-1 based on lifetime&gt;<br> example: time1 0.5<br> default: 1<br>

</dd> <dt>colour1</dt> <dd>

The colour at stage 1.<br>     format: colour1 &lt;r&gt; &lt;g&gt; &lt;b&gt; \[&lt;a&gt;\]<br> example: colour1 0 1 0 1<br> default: 0.5 0.5 0.5 0.0<br>

</dd> <dt>time2</dt> <dd>

The point in time of stage 2.<br>     format: time2 &lt;0-1 based on lifetime&gt;<br> example: time2 1<br> default: 1<br>

</dd> <dt>colour2</dt> <dd>

The colour at stage 2.<br>     format: colour2 &lt;r&gt; &lt;g&gt; &lt;b&gt; \[&lt;a&gt;\]<br> example: colour2 0 0 1 1<br> default: 0.5 0.5 0.5 0.0<br>

</dd> </dl>

@note The number of stages is variable. The maximal number of stages is 6; where time5 and colour5 are the last possible parameters.

To create a colour interpolation affector, include a section like this within your particle system script:

```cpp
affector ColourInterpolator
{
    // Settings go here
}
```

## ColourImage Affector {#ColourImage-Affector}

This is another affector that modifies the colour of particles in flight, but instead of programmatically defining colours, the colours are taken from a specified image file. The range of colour values begins from the left side of the image and move to the right over the lifetime of the particle, therefore only the horizontal dimension of the image is used.

Its extra attributes are:

<dl compact="compact">
<dt>image</dt> <dd>

The start of a range of rotation speed to be assigned to emitted particles.<br>     format: image &lt;image\_name&gt;<br> example: image rainbow.png<br> default: none<br>

</dd> </dl>

To create a ColourImage affector, include a section like this within your particle system script:

```cpp
affector ColourImage
{
    // Settings go here
}
```

## DeflectorPlane Affector {#DeflectorPlane-Affector}

@copybrief Ogre::DeflectorPlaneAffector

The attributes are:

<dl compact="compact">
<dt>plane\_point</dt> <dd>

A point on the deflector plane. Together with the normal vector it defines the plane.<br> default: plane\_point 0 0 0<br>

</dd> <dt>plane\_normal</dt> <dd>

The normal vector of the deflector plane. Together with the point it defines the plane.<br> default: plane\_normal 0 1 0<br>

</dd> <dt>bounce</dt> <dd>

The amount of bouncing when a particle is deflected. 0 means no deflection and 1 stands for 100 percent reflection.<br> default: bounce 1.0

</dd> </dl>

## DirectionRandomiser Affector {#DirectionRandomiser-Affector}

@copybrief Ogre::DirectionRandomiserAffector

Its extra attributes are:

@par randomness
@copybrief Ogre::DirectionRandomiserAffector::setRandomness
@par
Default: randomness 1

@par scope
@copybrief Ogre::DirectionRandomiserAffector::setScope
@par
Default: scope 1.0

@par keep_velocity
@copybrief Ogre::DirectionRandomiserAffector::setKeepVelocity
@par
Default: keep_velocity false

## TextureAnimator Affector {#TextureAnimator-Affector}

@copybrief Ogre::TextureAnimatorAffector

Its extra attributes are:

@par texcoord_start
@copybrief Ogre::TextureAnimatorAffector::setTexcoordStart
@par
Default: texcoord_start 0

@par texcoord_count
@copybrief Ogre::TextureAnimatorAffector::setTexcoordCount
@par
Default: texcoord_count 0
@copydetails Ogre::TextureAnimatorAffector::setTexcoordCount

@par duration
@copybrief Ogre::TextureAnimatorAffector::setDuration
@par
@copydetails Ogre::TextureAnimatorAffector::setDuration
@par
Default: duration 0

@par random_offset
@copybrief Ogre::TextureAnimatorAffector::useRandomStartOffset
@par
Default: random_offset false