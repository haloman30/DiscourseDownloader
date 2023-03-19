![DiscourseDownloader](./_assets/discoursedl_logo_large.png)

DiscourseDownloader is an application intended to archive/mirror a Discourse forum as
completely as possible. This is done by leveraging the Discourse API and preserving
as much content as possible.

This project is available on both [GitHub](https://github.com/haloman30/DiscourseDownloader)
 and the [Elaztek Studios Gitlab](https://gitlab.elaztek.com/haloman30/discoursedownloader).


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

> **Note:** Some features listed below are currently incomplete or under development and may
> not function as expected. Please see the Development Status section below for information
> on what features are currently present.

DiscourseDownloader has two primary components - the JSON downloader and the HTML builder.

##### JSON Downloader

The JSON downloader is the first step in the archive process. Using Discourse's API, it goes
through and collects any and all requested data and compiles it into a series of JSON files.
Many of these files are identical to those served by the Discourse API.

Storing the data in this format first has a couple key benefits over simply creating HTML files first:

* **More Complete Archives** - Storing virtually all of the information available in the Discourse
  API in its original format helps to ensure that the resulting archives are as complete as possible.
  Even in cases where a viewable website might not be available, the JSON data by itself contains
  all of the real content of the forum.
* **Easy HTML Rebuilding** - Storing the JSON data directly also allows for easy rebuilding of an
  HTML archive website later, allowing for any improvements or changes to be made to how the web
  archive looks and feels - all without having to redownload any content from the original forum.
  This is particularly useful in cases where a download is performed prior to a forum's closure,
  such as the case with HaloWaypoint - where redownloading the content is simply not possible.

The general process the downloader goes through is as follows:
1. Build a list of all categories
2. For each category, build a complete list of topic URLs
3. Download each topic, taking care to download any additional posts as by default, the API
   will only provide the first 20 posts with the topic information
4. Depending on configuration, perform either a basic or in-depth sanity check:
   * **Basic** - The basic sanity check simply ensures that all topic/post counts line up with what
     was originally pulled from the API. This is a quicker check, but doesn't check anything on-disk.
   * **In-Depth** - An in-depth sanity check goes through all data and ensures that each topic and
     post are actually present on disk. If any content is determined to be missing or incomplete
     during this check, the application will attempt to re-download this content again in order
     to avoid the necessity of running the entire download process again.
5. After the sanity checks have passed, the downloader will finish and the HTML builder will run, if
   the builder is enabled in the configuration.

Many features of the downloader can be controlled via the `website.cfg` file, which you can read
more about [here](https://haloman30.com/projects/discoursedl/docs/?page=config).

##### HTML Builder

The HTML builder is the second step in the archive process. It goes through all content and creates
a fully functional website from the downloaded JSON data, designed to be as easy to use as possible.

> **The HTML Builder is not yet implemented.**

### Development Status

The current progress of the application is as follows:

* [x] JSON Downloader
  * [x] Categories and Topics
    * [x] Categories
    * [x] Topics
    * [x] Posts
    * [x] Category images
    * [x] Tags
    * [ ] Tag groups
  * [x] User Profiles
    * [x] Profile
    * [x] Badges
    * [x] Avatars
    * [x] User Actions
    * [ ] Private Messages (will require auth)
  * [x] Miscellaneous Site Information
    * [x] Site Info
    * [x] Groups
    
* [ ] HTML Builder

Currently, the application is capable of downloading forum topic and post content in JSON format.

### Credits

Some content, such as HTML templates and certain code snippets, are heavily based on those found in
ArchiveDiscourse, found [here](https://github.com/kitsandkats/ArchiveDiscourse).

Some code, such as most of what exists in `/components/utils/` and certain classes, such as
`BlamConfigurationFile` and `BlamColor` are pulled from the Blamite Game Engine, developed by
Elaztek Studios, found [here](https://elaztek.com/projects/blamite).