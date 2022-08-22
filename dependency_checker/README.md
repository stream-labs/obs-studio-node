# Dependency checking

It will compare current list of dependencies to a list saved previously. 

Check done by `check_dependencies.cmd` script. If there any changes it will fail a build.

Example of saved list `obs-studio-server\dependencies\obs64.exe.txt`

List prepared by `dumpbin /DEPENDENTS` tool.


## Usage 

When a check had failed it will print to a console the name of a binary which have changed dependencies. 
```
  Check dependecy for color_picker.node

  InputObject        SideIndicator
  -----------        -------------
          dwmapi.dll <=


  !! !! !! !! !! !!
    Dependencies changed for color_picker.node
    Saved dependencies C:/work/repos/color-picker/dependencies/color_picker.node.txt
    New dependencies C:/work/repos/color-picker/build/color_picker.node.txt
    More info https://github.com/stream-labs/obs-studio-node/blob/staging/dependency_checker/README.md
  !! !! !! !! !! !!
```

To make a build working again. You need to review changes in a list of dependencies. If changes was not intentional or not needed then fix it by updating cmake. If changes was intentional then copy updated list to saved list and include this change in your PR.

Check will be skipped for Debug target. As this target can have different names of linked libraries and we intrested in changes in Release mode.

## Integration
### Simple way is to add sustom post build command 

Example:
https://github.com/stream-labs/a-files-updater/blob/5ef0900cd05c330988a95a87053fe0bf29e0bddf/CMakeLists.txt#L123

## Advanced way additional build target

If there is too many binaries to check it will take noticable amount of time. 

So better to move a check to a separate target. 

Example:
https://github.com/stream-labs/obs-studio/blob/03442ce67f35566963bc73a1ad00be2ccb9e50e5/CMakeLists.txt#L320

And it be run like 
`cmake --build build --target check_dependencies --config RelWithDebInfo`