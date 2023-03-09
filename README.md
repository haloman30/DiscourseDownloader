# DiscourseDownloader

DiscourseDownloader is an application intended to archive/mirror a Discourse forum as
completely as possible. This is done by leveraging the Discourse API and preserving
as much content as possible.


### Development Status

Currently, this project is **extremely** work-in-progress. It exists publicly for now
so that those interested may have a look, but nothing is set-in-stone and there will likely
be bugs or unfinished functionality. Expect regular commits until an initial "release".

### History

This project was inspired by [ArchiveDiscourse](https://github.com/kitsandkats/ArchiveDiscourse),
a Python script created by mcmcclur and adapted by kitsandkats for archiving a Discourse forum.

However, the script was imperfect and had a few odd bugs I had to work out - and by default wouldn't
download *everything*. Additionally, it was a fairly basic script - and wasn't intended for the
creation of a true "mirror" of a website.

With the closure of HaloWaypoint's forums announced in March 2023, I found no other suitable
option for archival - WinHTTrack had a great deal of trouble, and wget, while doing well, was
completely outpaced by the original Python script. This is in large part due to the sheer volume
of content on HaloWaypoint - with topic counts approaching 500k.

And so, I decided to build my own archival application from the ground up - and of course release
it into the wild for others to give a spin (or improve it should they wish).

### Features and Functionality

TBA

### Credits

Some content, such as HTML templates and certain code snippets, are heavily based on those found in
ArchiveDiscourse, found [here](https://github.com/kitsandkats/ArchiveDiscourse).

Some code, such as most of what exists in `/components/utils/` and certain classes, such as
`BlamConfigurationFile` and `BlamColor` are pulled from the Blamite Game Engine, developed by
Elaztek Studios, found [here](https://elaztek.com/projects/blamite).