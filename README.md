# CGeoIPModule
 _Geoblocking module for IIS written in native C++_
 
 This is my first real dive into C++! If you notice ive done something incorrectly within this code, please do point it out or make a PR!
 
## Installation

- copy `CGeoIPModule.xml` to `%windir%\System32\inetsrv\config\schema`

- edit `%windir%\System32\inetsrv\config\applicationHost.config` and find the line

```
        <sectionGroup name="system.webServer">
```
and add `<section name="CGeoIPModule" />` 

## Usage

action options
 - `Not Found`
 - `Abort`
 - `Forbidden`
 - `Unauthorized`
 - `Reset`
 
mode options
 - `block`
 - `allow`
 
Server variable `GEOIP_COUNTRY` will contain the iso country code on successful lookup, `ZZ` for local addresses as defined by the `IsLocalAddress()` function or `--` for unsuccessful lookups

example usage `web.config` to block one country
```xml
    <system.webServer>
    ...
        <CGeoIPModule enabled="true" action="Not Found" allowListed="false" path="C:\GeoIP\GeoIP-country.mmdb">
            <CountryCodes>
                <clear />
                <add code="CN" />
            </CountryCodes>
        </CGeoIPModule>
        ...
```

example usage `web.config` to allow only one country
```xml
    <system.webServer>
    ...
        <CGeoIPModule enabled="true" action="Not Found" allowListed="true" path="C:\GeoIP\GeoIP-country.mmdb">
            <CountryCodes>
                <clear />
                <add code="US" />
            </CountryCodes>
        </CGeoIPModule>
        ...
```

or perhaps you just want the server variable to be set for easy retireval from server scripting, just set allowListed to false with no country code's selected
```xml
    <system.webServer>
    ...
        <CGeoIPModule enabled="true" allowListed="false" path="C:\GeoIP\GeoIP-country.mmdb">
            <CountryCodes>
                <clear />
            </CountryCodes>
        </CGeoIPModule>
        ...
```

## Building

- Get libmaxminddb
```
git clone --recursive https://github.com/maxmind/libmaxminddb
mkdir libmaxminddb\build && cd libmaxminddb\build
cmake -DMSVC_STATIC_RUNTIME=ON -DBUILD_SHARED_LIBS=OFF ..
cmake --build . --config Release --target install
```

- Download and install Windows SDK.

- Build project with Visual Studio (I use 2022)

## Reason for this modules existence
I have been using the [GeoIP2blockModule](https://github.com/RvdHout/IIS-GeoIP2block-Module) for several years, and while its a well written solution, it is a bit slower (probably due to the introduction of asp.net into the request pipeline)

This module has very little impact on IIS's performance, making it more suitable to sites that see heavy load. Though it does lack a nice UI in IIS manager