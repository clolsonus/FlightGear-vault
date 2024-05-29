# FlightGear-vault

These are the earliest and most historic versions of FlightGear I found in my personal archives.

You may find a few things here interesting if you are a FlightGear fan.

## History and Purpose of FlightGear

In 4th-fgfs/docs/fltgear.ps (postscript ... aka adobe viewer, gnome evince,
etc.) is a telling of the FlightGear goals as we understood them at the
beginning of the project.

Then there is a brief status report.  All these years later I had forgotten it
was Eric Korpela who did most of the initial work to setup a code organization
scheme and began crafting low level 2d libraries that were multi-platform
(windows, linux, sun, sgi, mac, etc.) and presumably could be optimized to run
fast on all these platforms.  The intent was to then build our own 3d graphics
library on top of the 2d library.  Also thanks to Michael Basler for putting
together this document at the time.

## Then what happened after this code?

I didn't set out to be a trouble maker, but the development plan wasn't
proceeding forward enough to my liking.  It felt stalled out on low level
graphics library work, and after an initial burst of interest and activity, it
suddenly felt in early 1997 like we weren't getting anywhere.

So as I recall (which is probably at least somewhat wrong) I decided on a
different direction.  I was aware of OpenGL through my university class work
(Yeah U of MN Gophers!) and aware it ran on SunOS and SGI.  It also was becoming
available on PC's (both windows and Linux) and MacOS as well.  This checked the
multi-platform requirement box, and also would allow us to leap frog the low
level graphics development requirement which beginning to look more and more
like it was holding us back.  (Not in terms of technical merits or code quality,
just in terms of pace and in terms of reinventing something we didn't need to
reinvent.)

As I recall (sorry very likely imperfectly) I set out to build a proof of
concept simulator using OpenGL and some real terrain data I found.  I think this
was pre-SRTM availability in 1997.  So I spent quite a bit of time taking raw
terrain data and generating a 3d mesh, forming it into an extensible whole world
coordinate system, and tiling it so terrain chunks could be paged in and out. In
addition I found LaRCsim which implemented a Navion flight dyanmics model and
incorporated that into the system.

When this was working well enough to run and show, I shared it with the larger
project.  At that point not much further progress had been made with the 2d
graphics system and the original FlightGear code base.  My sales pitch was
something along the lines of:

Guys, we've done a lot of great work, but I don't think we are going in the
right direction.  I don't want to just complain and make everyone feel bad about
themselves, so instead I took some time off and built an alternative that I
think is going to be better.  I'm not trying to do a hostile take over or be
negative about anyone's work, but I would like to go in this other direction and
maybe some of you will find this interesting and join me.  I felt bad about
leaving the original effort in the rearview mirror and stealing all the energy
for myself.

If I haven't done this already many times before, or if the communication never
made it where it needed to go, I apologize to David Murr and Eric Korpela and
anyone else who heavily invested months into that original project!

## Why make this available now in 2024?

I just recently stumbled on this code in a Projects directory and had forgot I
still had it.  Other copies may be out there and I'm probably just duplicating
that, but in case this contains different snapshots in time, or is in anyway
interesting to anyone, I wanted to share it and make it available.  Sometimes it
is good to look back on the journey and see how far we have all come!