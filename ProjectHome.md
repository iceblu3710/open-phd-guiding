The PHD Guiding software from [Stark Labs](http://www.stark-labs.com) is a popular, free package for telescope autoguiding on Macs and PCs.  The goal of this project is to bring PHD Guiding to developers on Linux, OS X, and Windows.

Trunk holds the code for PHD2.  This is a major refactor of the PHD codebase which brings a new internal architecture, support for Adaptive Optics devices and a new user interface.

The Linux PHD 1.0 port lives in branches/openphd1. This has the core routines of PHD Guiding as well as hardware that is supported on Linux.  Heavy use is made of the INDI platform on Linux, much as heavy use of ASCOM is made on Windows.

In addition to the trunk, there is a "craig" branch.  This branch has the main PHD1 Guiding releases under OS X and Windows.  Builds of this will give you a current and nearly complete build of PHD Guiding on either platform (some hardware has been left out for licensing reasons).

By opening up the core guide routines for PHD Guiding, we open ourselves up to improvement as PHD's original developer (Craig Stark) can only do so much by himself.

For all discussion, please use the [Discussion group](https://groups.google.com/forum/?fromgroups#!forum/open-phd-guiding).