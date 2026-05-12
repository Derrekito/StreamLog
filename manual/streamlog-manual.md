---
title: "StreamLog Manual"
subtitle: "A Lightweight C++ Logging Library"
author: "Derrek Landauer"
date: \today
toc: true
secnum: true
acronyms: false
license: "MIT"
---

# Introduction

!include-headless ../README.md

# Architecture

!include-headless ../docs/architecture.md

# Usage Guide

This section covers how to use StreamLog effectively in your applications. The topics progress from fundamental concepts like log levels and their semantics to advanced customization through class inheritance. Understanding log levels and compile-time filtering is essential for all users, while the customization section is primarily for developers who need to extend the library's behavior. Each subsection includes practical code examples that demonstrate the concepts in context.

## Log Levels

!include-headless ../docs/usage/log-levels.md

## Customization

!include-headless ../docs/usage/customization.md

# Development

This section is for developers who want to build StreamLog from source, modify the library, or contribute to the project. It covers the build system in detail, including platform-specific configurations and the modular Makefile structure. The information here is not needed for simply using the library; users who just want to integrate StreamLog into their projects can skip to the Reference section. Developers working on embedded platforms or custom build environments will find the platform configuration details particularly relevant.

## Building

!include-headless ../docs/development/building.md

# Reference

This section provides comprehensive reference documentation for StreamLog. The API Reference documents all public functions, classes, and macros with their signatures and usage examples. The Configuration section details all build-time and runtime options that affect library behavior. The Troubleshooting section addresses common issues and provides debugging techniques for when things don't work as expected.

## API Reference

!include-headless ../docs/reference/api.md

## Configuration

!include-headless ../docs/reference/configuration.md

## Troubleshooting

!include-headless ../docs/reference/troubleshooting.md
