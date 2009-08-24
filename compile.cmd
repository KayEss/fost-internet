@echo off
echo fost-internet
..\bjam toolset=msvc %* && ..\bjam toolset=msvc examples %*
