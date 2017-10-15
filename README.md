# PluginColorCursor

Creates the ColorCursor.dll plugin for Rainmeter.

Build in conjunction with the Rainmeter Plugin SDK from [Rainmeter Plugin SDK](https://github.com/rainmeter/rainmeter-plugin-sdk)  
Documentation for the SDK at [Developers Documentation](https://docs.rainmeter.net/developers/#CreatePlugin)

## Options

### Format
`Format=RGB` Returns rrr,ggg,bbb formatted rgb color  
`Format=Red` Returns rrr red color element  
`Format=Green` Returns ggg green color element  
`Format=Blue` Returns bbb blue color element  

### RealTime

`RealTime=0` (default) Plugin will do nothing until called with !CommandMeasure  
`RealTime=1` Plugin will constantly return the color of the pixel under the mouse cursor

## Commands 

`[!CommandMeasure MeasureName "GetColor"]`

When executed in an action, the plugin will return the color of the pixel under the mouse cursor
