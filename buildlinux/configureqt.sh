#!/bin/bash
./configure -debug -developer-build -opensource -nomake examples -nomake tests -skip qtconnectivity -skip qtwebengine -openssl $1
