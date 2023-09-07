# HTS221

A Particle library for HTS221

## Welcome to your library!

To get started, modify the sources in [src](src). Rename the example folder inside [examples](examples) to a more meaningful name and add additional examples in separate folders.

To compile your example you can use `particle compile examples/usage` command in [Particle CLI](https://docs.particle.io/guide/tools-and-features/cli#update-your-device-remotely) or use our [Desktop IDE](https://docs.particle.io/guide/tools-and-features/dev/#compiling-code).

Libraries can also depend on other libraries. To add a dependency use [`particle library add`](https://docs.particle.io/guide/tools-and-features/cli#adding-a-library) or [library management](https://docs.particle.io/guide/tools-and-features/dev/#managing-libraries) in Desktop IDE.

After the library is done you can upload it with `particle library upload` or `Upload` command in the IDE. This will create a private (only visible by you) library that you can use in other projects. If you wish to make your library public, use `particle library publish` or `Publish` command.

_TODO: update this README_

## Usage

Connect XYZ hardware, add the HTS221 library to your project and follow this simple example:

```
#include "HTS221.h"
HTS221 hTS221;

void setup() {
  hTS221.begin();
}

void loop() {
  hTS221.process();
}
```

See the [examples](examples) folder for more details.

## Documentation

TODO: Describe `HTS221`

## Contributing

Here's how you can make changes to this library and eventually contribute those changes back.

To get started, [clone the library from GitHub to your local machine](https://help.github.com/articles/cloning-a-repository/).

Change the name of the library in `library.properties` to something different. You can add your name at then end.

Modify the sources in <src> and <examples> with the new behavior.

To compile an example, use `particle compile examples/usage` command in [Particle CLI](https://docs.particle.io/guide/tools-and-features/cli#update-your-device-remotely) or use our [Desktop IDE](https://docs.particle.io/guide/tools-and-features/dev/#compiling-code).

After your changes are done you can upload them with `particle library upload` or `Upload` command in the IDE. This will create a private (only visible by you) library that you can use in other projects. Do `particle library add HTS221_myname` to add the library to a project on your machine or add the HTS221_myname library to a project on the Web IDE or Desktop IDE.

At this point, you can create a [GitHub pull request](https://help.github.com/articles/about-pull-requests/) with your changes to the original library. 

If you wish to make your library public, use `particle library publish` or `Publish` command.

## LICENSE
Copyright 2018 speirano

Licensed under the <insert your choice of license here> license
