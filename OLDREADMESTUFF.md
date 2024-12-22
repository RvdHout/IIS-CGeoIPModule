## Installation

- copy `CGeoIPModule.xml` to `%windir%\System32\inetsrv\config\schema`

- edit `%windir%\System32\inetsrv\config\applicationHost.config` and find the line

```xml
        <sectionGroup name="system.webServer">
```
and add `<section name="CGeoIPModule" />`

copy `CGeoIPModule.dll` to `%windir%\System32\inetsrv`

install via IIS dialogs (Server->Modules->Configure Native Modules->Register)

## Usage

action options
 - `Not Found`
 - `Close`
 - `Forbidden`
 - `Unauthorized`
 - `Reset`

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