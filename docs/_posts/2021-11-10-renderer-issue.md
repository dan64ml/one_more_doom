---
layout: post
title:  "Something went wrong"
date:   2021-11-10 22:18:03 +0300
---

I've started adding AI to monsters. First, very dirty version, showed unexpected problem. It looks like the renderer has a serious issue with mobj drawing.
Take a look at the video:

<iframe width="560" height="315" src="https://www.youtube.com/embed/uJvyqbRhAHM" title="YouTube video player" frameborder="0" 
allow="accelerometer; autoplay; clipboard-write; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>

The trooper "falls" under the floor, although the coordinates of the mobj are ok. The renderer definitely clips mobjs in a wrong way :(.
