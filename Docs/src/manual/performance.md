
Performance Hints {#performance}
=================

DO:

-   **Sort your creation order of Entities by frequency of creation &
    destruction (LIFO):** Entities that will be created & destroyed
    often should be done last. Entities that will never be destroyed
    should be created first, followed by Entities that will rarely be
    removed/destroyed.
-   **Use Instancing whenever possible:** InstancedObjects are
    lightweight, have less RenderQueue and API draw call overhead
-   **Reduce the number of bones in a rig to the minimum necessary.**
    Specially if there are going to be many copies of it
-   When destroying an entire level or exiting the game, it may be
    advisable to disable [cleanups](#3.2.1.Cleanups|outline). Unless
    your destroy procedure is respecting LIFO order, multiple
    unnecessary cleanups may be triggered, slowing the shutdown routine.
    If a level is being destroyed but the game is still running,
    re-enable cleanups and perform one explicitly after having destroyed
    all objects involved.

DON'T:

-   Don't update objects created as `SCENE_STATIC` very often. Updating
    just one SceneNode or Entity will force to update many of them,
    possibly all; thus nullifying the performance benefits from
    `SCENE_STATIC`.
-   Don't create objects that never move, rotate or scale as
    `SCENE_DYNAMIC`
-   Don't create a Shadow Node that uses Render Queues that any of the
    previously executed `render_scenes` didn't use. eg. Setting the
    shadow node's pass to `rq_first 0` & `rq_last 5`; while the
    `render_scene` of the regular pass is just `rq_first 1 rq_last 4`
-   Don't specify more threads than the number of available cores. If
    your code fully uses (i.e.) 2 cores all for yourself, then substract
    2 to the number of threads (don't oversubscribe).[^11]

[^11]: Note: The minimum number of threads is 1. One CPU = One Thread.
    The SceneManager delegates its work on worker threads and goes to
    sleep.
