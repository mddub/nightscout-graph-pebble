## About this fork and me (andyrozman/urchin-cgm)

I am Android APS user and I have been using this watchface for a while now... I created very nice configuration which displays
all data I need. One of the lines I use is 'Active basal - NS Care Portal', which was actually implemented little bit wrong. It worked 
great for absolute values, but not for percent ones. This change fixes exactly that. So if you use TBR as percent, and use 'Active basal' 
status line, then this fix is for you. 
 
Here is link to compiled watchface [pbw file](https://raw.githubusercontent.com/andyrozman/urchin-cgm/master/release/urchin-cgm.pbw), so that you don't need to do it by yourself (it's little bit of hassle to make build work, since 
original Pebble site is no longer there, but you can use Rebble service instead).  
 


## Watchface

A Pebble watchface to view data from a continuous glucose monitor in graph format, like this:

![Screenshot](http://i.imgur.com/xefGk6A.png)

To install, enable Developer Mode in the Pebble app on your phone, then open [this pbw file][pbw] in the Pebble app.

Urchin CGM is an **U**nopinionated, **R**idiculously **C**onfigurable **H**uman **IN**nterface to your CGM. It's not released yet / in beta / a work-in-progress.

[![Circle CI](https://circleci.com/gh/mddub/urchin-cgm.svg?style=shield)](https://circleci.com/gh/mddub/urchin-cgm)

## Setup

* Open [the latest release][pbw] in your phone's browser, then open the file with the Pebble app to install.
* In the Pebble app on your phone, open the "Settings" screen for Urchin.
* To view data from a [Nightscout][cgm-remote-monitor] site, enter your site's URL.
* To view data from [Dexcom Share][dexcom-share], enter your username and password. (These credentials never leave your phone except to authorize with Dexcom's servers.)
* Optionally, personalize your watchface using the settings described below.

## Layout

The layout is one of the most Ridiculously Configurable aspects of Urchin. The settings page includes a handful of pre-set layouts to get you started:

![](http://i.imgur.com/o8hLgv9.png) ![](http://i.imgur.com/c81Yr4R.png) ![](http://i.imgur.com/KMhoWEv.png)

Use one as a starting point to build your own watchface: reorder the elements, change heights and colors, toggle borders, move the icons...

## Graph

Set the bounds and target range of the graph on your phone.

You can choose the shape and size of the points on the graph so that the glucose history is as long or as short as you want. To show up to 12 hours of BGs, plot the points super-thin or overlapping. For a "sparkline" view of the last hour, plot bigger points with more space in between.

![](http://i.imgur.com/D91OqLn.png) ![](http://i.imgur.com/7RBWAhe.png)

## Status bar content

The status bar can display content from a variety of sources:

* **Date** - in whatever format you want. (e.g. `Sat Oct 7`)
* **Uploader battery level** - if your Nightscout data comes from a wired rig/xDrip. (e.g. `36%`)
* **Raw Dexcom readings** - [raw sensor readings][raw-dexcom-readings] plus noise level. (e.g. `Cln 97 104 106`)
* **Uploader battery, Dexcom raw** - combination of the above two. (e.g. `36% Cln 97 104 106`)
* **Active basal - NS Care Portal** - the currently-active basal rate based on treatments in [Nightscout Care Portal][care-portal]. If a temp basal is currently active, shows the difference from normal basal and how many minutes ago the temp basal began. (e.g. `(19) 1.5U/h +0.6`)
* **Insulin on board** - this can be calculated from treatments entered manually in [Nightscout Care Portal][care-portal], or reported automatically from [MiniMed Connect][minimed-connect], [Loop][loop], or [OpenAPS][openaps-status-uploads]. (e.g. `2.3 U`)
* **Insulin + carbs on board** - same IOB as above plus carbs-on-board entered in Care Portal. (e.g. `2.3 U  31 g`)
* **Loop status** - status from the [Loop][loop] iOS app -- predicted BG, IOB, COB, current temp basal, pump battery, pump reservoir, phone battery -- in whatever format you want. (e.g. `143 0.2U 13g 1.50`, or `1.50U/h 0.2 U 13 g | 1.57v 109U 83%`)
* **IOB and temp - OpenAPS** - IOB and currently-active temp basal rate from the most recent [OpenAPS status upload][openaps-status-uploads], or if the most recent status indicates failure, the time since that failure plus the time and IOB from the last successful status. (e.g. `(2) 1.1U 1.9x13` or `(4) -- | (+23) 2.2U`)
* **Custom URL - text** - if you want to summarize your data in a custom way.
* **Custom URL - JSON** - show custom text, with support for a `timestamp` field to display recency (e.g. `(3) your text`).
* **Custom text** - remind yourself whose glucose readings you're looking at, or leave a terse inspirational message.
* **Multiple** - Up to 3 of the above, one on each line.

The recency indicator in the status bar is configurable. You can choose the format (`(3)`, `3:`, etc.) and the conditions under which it is shown (for example, "don't show the recency as long as it's fewer than 10 minutes old," or "don't show the status at all when it's more than 30 minutes old")

## Pump data

If you are using Nightscout to track data from an insulin pump, you may choose to display **bolus history** (as ticks) and/or **basal history** (as a bar graph):

![](http://i.imgur.com/UmQ5Yqx.png)

To enter pump data manually, you can use [Nightcout Care Portal][care-portal] or the ["CarePortal" Pebble app][pebble-care-portal].

To upload pump data automatically, you can use [RileyLink][rileylink_ios] or [Loop][loop] on iOS, or build an [OpenAPS uploader][openaps].

## Predictions

Like pump data, predicted future BGs can be plotted if you are using [Loop][loop] on iOS, or [OpenAPS][openaps] with the Advanced Meal Assist (AMA) feature of the oref0 algorithm. For example, AMA generates three different projections assuming no carbs, normal carb absorption, and accelerated carb absorption.

![](http://i.imgur.com/eaR84wh.png) ![](http://i.imgur.com/BSed3ru.png)

## What do the icons mean?

The data that you see on your watch travels like this: `Sensor/Receiver -> Server -> Phone -> Pebble`.

![](http://i.imgur.com/8D9uIWo.png) There is a problem with the Phone -> Pebble connection: it's been a while since your watch has been able to reach your phone. Maybe your phone is out of range. Maybe it's on airplane mode. Maybe you need to charge your phone.

![](http://i.imgur.com/cNiLolr.png) There is a problem with the Server -> Phone connection: it's been a while since your phone has been able to reach the server. Maybe your phone's network connection is bad. Maybe your Nightscout server / Dexcom's server is down.

![](http://i.imgur.com/gfaaZnQ.png) There is a problem with the Sensor/Receiver -> Server connection: the latest data on the server is old. Maybe there's a problem with your receiver or uploader. Maybe the sensor fell out.

When the watch fails to fetch data, a message describing the problem briefly appears in the graph.

## Contributing

Contributions are welcome in the form of bugs and pull requests. To report a bug or provide feedback, please [file an issue][file-issue]. To contribute code, please use the instructions below to build and test the watchface.

* Install the [Pebble SDK Tool].

* Install and activate the Pebble SDK. As of this writing, the app is built with SDK 3.14, but later versions should work, too.
  ```
  pebble sdk install 3.14
  pebble sdk activate 3.14
  ```

* Build and run the watchface with a command like:
  ```
  pebble build && pebble install --emulator basalt && pebble logs
  ```

* The watchface will ask for settings from the "phone." Open the configuration page with this command and hit "save" in your browser (you'll need to do this only once):
  ```
  pebble emu-app-config --emulator basalt
  ```

* At some point you'll want to install your revisions on your watch. Flip the "Enable Developer Connections" switch in the Pebble app on your phone, then:
  ```
  pebble install --phone <phone ip address>
  ```

**Tips:**

* **Testing the configuration page**: For a default build, the contents of `config/index.html` and its dependencies are inlined and converted into a data URI. This data URI is rebuilt and included in the JS during a `pebble build`. When testing changes to the configuration page, however, it's easier if the emulator opens the un-inlined version. To build the watchface to open the HTML file, set `BUILD_ENV` to `development`:
  ```
  BUILD_ENV=development pebble build
  pebble install --emulator basalt
  pebble emu-app-config --emulator basalt
  ```

* **Debug logging:** `DEBUG=true` enables extra logging from the watchface and JS:
  ```
  DEBUG=true pebble build
  pebble install --emulator basalt
  pebble logs --emulator basalt
  ```

* **Syntax checking:** If you use Vim, I highly recommend using [Syntastic] to display syntax errors. On my OS X system running Pebble Tool v4.1.1, these lines make Syntastic aware of Pebble's header files and suppress warnings generated within those files:

  ```
  let g:syntastic_c_include_dirs = ['/Users/<user>/Library/Application Support/Pebble SDK/SDKs/current/sdk-core/pebble/aplite/include', 'build/aplite']
  let g:syntastic_c_remove_include_errors = 1
  ```

* **JavaScript errors:** If you see a JavaScript error in the console, the line numbers will be reported relative to `build/pebble-js-app.js`, which is the concatenation of all files in `src/js/**/*.js`.

## Testing

Since this software displays real-time health data, it is important to be able to verify that it works as expected.

The most effective method of integration testing I've found is to [compare screenshots][screenshots-artifact]. This relies on [ImageMagick] to compute diffs. Screenshot tests and JavaScript unit tests are run automatically by CircleCI.

* **Install testing dependencies**

  Install [ImageMagick], then use `pip` to install Python testing dependencies:
  ```
  pip install -r requirements.txt --user
  ```

* **Use the live-reload tool**

  This is the fastest way to test the watchface against specific configuration and data.

  Create a new test class in `test/test_screenshots.py`:
  ```python
  class TestSomething(ScreenshotTest):
      config = {...}
      def sgvs(self):
          return [...]
  ```

  Build and install the watchface:
  ```
  pebble build && pebble install --emulator basalt
  ```

  Start the live-reload tool:
  ```
  . test/live_reload.sh TestSomething
  ```

  Whenever `test/test_screenshots.py` is modified, the `config` property of the test will be sent to the watchface. When the watchface makes a request for new data, the corresponding `sgvs()`/`treatments()`/etc. method on the test will be evaluated.

* **Run the screenshot test suite locally**
  ```
  . test/do_screenshots.sh
  ```

* **Run an individual screenshot test locally**
  ```
  . test/do_screenshots.sh -k TestName
  ```

* **Use the mock Nightscout server directly**

  Start the server:
  ```
  MOCK_SERVER_PORT=5555 python test/server.py
  ```

  Build the watchface as usual:
  ```
  pebble build && pebble install --emulator basalt && pebble logs
  ```

  Use an editor to save mock data, send it to the server, verify it:
  ```
  vi sgv-data.json
  # ...edit mock data...

  # POST it for the server to store
  curl -d @sgv-data.json http://localhost:5555/set-sgv

  # Verify:
  curl http://localhost:5555/api/v1/entries/sgv.json

  # ("sgv" can be sgv, entries, treatments, devicestatus, or profile)
  ```

  Use the browser to configure the watchface:
  ```
  # Make sure you set the Nightscout host to "http://localhost:5555"
  pebble emu-app-config --emulator basalt
  ```

* **Run JavaScript unit tests locally**

  These require [Node]. See the [Mocha] and [Expect] docs.

  ```
  cd test/js
  npm install
  npm test
  ```

## To do:
* High/low BG alerts
* Stale data alerts
* More color configurability
* A fixed layout which supports Pebble Time Round
* More dynamic sizing of content (e.g. bigger/smaller time and BG)
* etc.

## Disclaimer

This project is intended for educational and informational purposes only. It is not FDA approved and should not be used to make medical decisions. It is neither affiliated with nor endorsed by Dexcom.

[care-portal]: http://www.nightscout.info/wiki/welcome/website-features/cgm-remote-monitor-care-portal
[cgm-remote-monitor]: https://github.com/nightscout/cgm-remote-monitor
[dexcom-share]: http://www.dexcom.com/apps
[Expect]: https://github.com/Automattic/expect.js
[file-issue]: https://github.com/mddub/urchin-cgm/issues
[ImageMagick]: http://www.imagemagick.org/
[loop]: https://github.com/LoopKit/Loop
[minimed-connect]: http://www.nightscout.info/wiki/welcome/website-features/funnel-cake-0-8-features/minimed-connect-and-nightscout
[Mocha]: https://mochajs.org/
[Node]: https://nodejs.org/
[openaps]: https://github.com/openaps/docs
[openaps-status-uploads]: http://openaps.readthedocs.io/en/latest/docs/walkthrough/phase-1/nightscout-setup.html
[pbw]: https://raw.githubusercontent.com/mddub/urchin-cgm/master/release/urchin-cgm.pbw
[Pebble SDK Tool]: https://developer.getpebble.com/sdk/
[pebble-care-portal]: https://apps.getpebble.com/en_US/application/568fb97705f633b362000045
[raw-dexcom-readings]: http://www.nightscout.info/wiki/labs/interpreting-raw-dexcom-data
[rileylink_ios]: https://github.com/ps2/rileylink_ios
[screenshots-artifact]: https://circleci.com/api/v1/project/mddub/urchin-cgm/latest/artifacts/0/$CIRCLE_ARTIFACTS/output/screenshots.html
[Syntastic]: https://github.com/scrooloose/syntastic
