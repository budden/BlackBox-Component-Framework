# Overview over the BlackBox Component Builder #

The BlackBox Component Builder is an integrated development environment optimized for component-based software development. It consists of development tools, a library of reusable components, a framework that simplifies the development of robust custom components and applications, and a run-time environment for components.

In BlackBox, the development of applications and their components is done in Component Pascal. This language is a descendant of Pascal, Modula-2, and Oberon. It provides modern features such as objects, full type safety, components (in the form of modules), dynamic linking of components, and garbage collection. The entire BlackBox Component Builder is written in Component Pascal: all library components, all development tools including the Component Pascal compiler, and even the low-level run-time system with its garbage collector. In spite of its power, Component Pascal is a small language that is easy to learn and easy to teach.

The component library that comes with BlackBox contains components for user interface elements such as command buttons or check boxes; various components that provide word processing functionality (Text subsystem); various components that provide layout management functionality for graphical user interfaces (Form subsystem); database access components (Sql subsystem); communication components (Comm subsystem); and a number of development tool components such as compiler, interface browser, debugger, and so on (Dev subsystem).

Component interactions are governed by the BlackBox Component Builder's Frameworks. These consist of a number of complementary programming interfaces. These interfaces are much simpler and safer, and platform-independent moreover, than basic APIs, such as the Windows APIs. For interactive applications, they define a quite unique compound document architecture. This architecture enables rapid application development (RAD), including the rapid development of new user interface components. The framework design strongly emphasizes robust component interaction. This is important for large-scale software projects that involve components from different sources and evolution of the software over long periods of time. To combine the productivity of a RAD environment with a high degree of architectural robustness was a major design goal for the BlackBox Component Builder. It was attempted to create an environment that is light-weight and flexible, yet doesn't sacrifice robustness and long-term maintainability of software produced with it. This was made possible by an architecture that decomposes the system into components with well-defined interfaces. Software is evolved incrementally, by adding, updating, or removing entire components.

The BlackBox run-time environment supports dynamic linking and loading (and unloading) of components. In this way, a system can be extended at run-time, without recompiling, relinking, or restarting existing code. Component objects (i.e., instances of classes contained in components) are automatically removed when they are not referenced anymore. This garbage collection service is a crucial safety feature of the run-time system, since it allows to prevent errors like memory leaks and danging pointers, which are almost impossible to avoid in a heavily component-oriented system like BlackBox.


# Aim #

The aim of this project is providing of core framework as platform for executing software components written in Component Pascal.
Currently we have only one publicly available open-source implementation (Windows, 32-bit). Hopefully, it will be truly cross-platform at some day.


# Notes #

## Download ##

The original open source code was taken from distribution of BlackBox Component Builder 1.6 RC6 at [www.oberon.ch](http://www.oberon.ch/blackbox.html)

## To developers ##

This distribution doesn't include neither binary executables nor other resource files. In order to compile it from sources, you need to have full installation of BlackBox Component Builder 1.6 RC6.


# License #

See file LICENSE for details (a modified "The Sleepycat License").
