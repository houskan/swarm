# Local methods for swarm navigation and formation

<div align="center">
<img src="gifs/debug.gif" alt="Demo" width="100%"/>
</div>

Niklaus Houska

*supervised by* Prof. Dr. Stelian Coros 

## Table of Contents
* [About the Project](#about-the-project)
* [Getting Started](#getting-started)
* [Usage](#usage)

## About the Project
This repository contains the code and Unreal Engine Project file for the implementation of my Bachelor Thesis at ETH Zurich. Read the [paper](https://github.com/houskan/swarm/blob/main/paper.pdf) for further information about the research and results.

### Abstract
This paper presents a local method for organizing a swarm of robots into a column formation.
The formation spots are defined by the integer coordinates of a two-dimensional
formation space known by each swarm member. The members are distributed over these
formation places only through local interaction and navigate using optimal reciprocal collision
avoidance. The method was successfully tested in simulation with robot numbers
between 4 and 200 and evaluated in its convergence time compared to an optimal global
solution, with the local method on average 50% slower. A correct end configuration is
reached independent of initial configuration and number of robots.

## Getting Started
The project is implemented in the game engine [Unreal Engine](https://www.unrealengine.com/en-US/) version 4.14. To build the game, an installation of the engine is required. Then download all resources from this repository and import UnitControl.uproject within the engine. To produce the Visual Studio files, right-click UnitControl.uproject -> Generate Visual Studio project files.

## Usage
| Controls | Input |
| --- | --- |
| Spawn robot | Left-mouse button |
| Delete robot | Middle-mouse button |
| Move formation | Right-mouse button (drag to determine orientation) |
| Toggle column spacing | S |
| Toggle column count | C |
<div align="center">
<img src="gifs/demo.gif" alt="Demo" width="100%"/>
</div>
