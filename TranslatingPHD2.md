# Introduction #

Translating PHD2 into different languages is a simple process.

First you will need a translation editor. [Poedit](http://poedit.net/) is an excellent free tool, and there are [several other choices](https://www.google.com/webhp?q=gettext+translations+editor) available too.

# Starting a new translation #

To translate PHD2 into a new language:

  1. Download the messages template file from the phd2 source repository: [messages.pot](https://open-phd-guiding.googlecode.com/svn/trunk/locale/messages.pot).
  1. In Poedit, select "Start a new translation", and open the `messages.pot` file you just downloaded.
  1. Save your work as `messages.po`.

# Updating an existing translation #

  1. Get the `messages.po` file for your language from the phd2 source repository in the [locale folder](https://code.google.com/p/open-phd-guiding/source/browse/#svn%2Ftrunk%2Flocale). Browse to the language sub-folder for your language and click on `messages.po`. Use the link on the right side of the page "View raw file" to save `messages.po` to your computer.
  1. Open the `messages.po` file in Poedit

# Viewing the translation in PHD2 #

When you save `messages.po`, Poedit will create `messages.mo` in the same folder. Copy `messages.mo` to your phd2 install folder

```
    <PHD2_Install_Folder>/locale/<YOUR_LANGUAGE_ABBREV>/messages.mo
```

Then, restart phd2 to see the updated translation.

# Publishing the translation #

Email the `messages.po` file to `andy.galasso@gmail.com`, or post a message on the [google group](https://groups.google.com/forum/?fromgroups=#!forum/open-phd-guiding) with `messages.po` as an attachment. We will commit the file to the PHD2 source repository, and the updated translation will become available in the next [development build](http://openphdguiding.org/snapshots.html) of PHD2.


---


## Notes for developers ##

If you make a change that affects translatable strings (strings added, removed, or modified), please run `build/locales_update.sh` to update the `messages.po` files for translators.

After receiving an updated `messages.po` file from translators, run `build/locales_compile.sh` to update the `messages.mo` files.