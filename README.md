# Closed Loop Experiment Manager (CLEM)
### A Simple and Inexpensive System for Multichannel, Closed Loop Electrophysiological Experimentation

<p>There is growing need for multichannel electrophysiological systems that record from and interact with neuronal systems in near real-time. Ideally, such systems would be inexpensive, reliable, user friendly, easy to set-up, open and expandable, and possess long life cycles in face of rapidly changing computing environments. Finally, they should provide powerful yet reasonably easy to implement facilities for developing closed-loop protocols for interacting with neuronal systems. </p>
<p>&nbsp;</p>
<p>Here we provide full sources for a solution we created referred to as Closed Loop Experiments Manager (CLEM). CLEM is a soft real-time, Microsoft Windows desktop application based on an inexpensive, general-purpose 64-channel data acquisition board (UEI PD2-MF-64-3M/12L) and a generic personal computer. CLEM provides a fully functional, user-friendly graphical interface, possesses facilities for recording, presenting and logging electrophysiological data from up to 64 analog channels, and facilities for interacting with external devices, such as stimulators, through digital and analog interfaces. Importantly, it includes facilities for loading “plugins” containing functions for running closed-loop protocols. Such plugins can be written in any programming language that can generate dynamic link libraries (DLLs) as long as they conform with CLEMs requirements.</p>
<p>&nbsp;</p>
<p>CLEM is a 64bit Windows desktop application written in C and C++ using Visual Studio 2013 and is based on the generic Win32 API and GDI. It thus does not require external code libraries, beyond those provided by the boards’ vendor.  The project generates a single executable file, which does not depend on registry entries but does require a configuration file (provided).
In addition to source files, the project contains setup files for installing an executable version.</p>
<p>&nbsp;</p>
<p>
Example code and further information needed for creating Plugins can be found in a related project:
<a href="https://github.com/Hananel-Hazan/CLEM-Plugin-Template">CLEM Plugin Template</a>
</p>

