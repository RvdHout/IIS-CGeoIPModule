#nullable disable
/* GeoblockConfigurationSection.cs
 *
 * Copyright (C) 2009 Triple IT.  All Rights Reserved.
 * Author: Frank Lippes, Modified for IIS 10 (.Net 4.6) by RvdH
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

using System;
using System.Collections.Generic;
using System.Net.Sockets;
using Microsoft.Web.Administration;

namespace CGeoIPModule
{
    /// <summary>
    /// This is the configuration object of the geoblock module. It is the connection between the module and the configuration section
    /// </summary>
    public sealed class GeoblockConfigurationSection : ConfigurationSection
    {
        /// <summary>
        /// The name of the geoblock configuration section
        /// </summary>
        public static string SectionName = "system.webServer/CGeoIPModule";

        /// <summary>
        /// Creates a new instance of the geoblock configuration section
        /// </summary>
        public GeoblockConfigurationSection()
        {
        }

        /// <summary>
        /// Specifies whether or not the module is enabled
        /// </summary>
        public bool Enabled
        {
            get { return (bool)base["enabled"]; }
            set { base["enabled"] = (bool)value; }
        }

        /// <summary>
        /// Specifies whether or not to use REMOTE_ADDR server variable to obtain client IP
        /// </summary>
        public bool RemoteAddr
        {
            get { return (bool)base["remoteAddr"]; }
            set { base["remoteAddr"] = (bool)value; }
        }

        /// <summary>
        /// Specifies the Deny Action Type
        /// </summary>
        public string Action
        {
            get { return (string)base["action"]; }
            set { base["action"] = (string)value; }
        }

        /// <summary>
        /// Points to the GeoIP.dat file
        /// </summary>
        public string Path
        {
            get { return (string)base["path"]; }
            set { base["path"] = (string)value; }
        }

        /// <summary>
        /// Whether the selected country codes are allowed or not
        /// </summary>
        public bool Allow
        {
            get { return (bool)base["allowListed"]; }
            set { base["allowListed"] = (bool)value; }
        }

        /// <summary>
        /// A collection that holds the selected country codes
        /// </summary>
        public CountryCollectionConfigurationElement CountryCodes
        {
            get 
            {
                return (CountryCollectionConfigurationElement)GetChildElement("countryCodes", typeof(CountryCollectionConfigurationElement)); 
            }
        }

        /// <summary>
        /// A collection that holds the exception rules
        /// </summary>
        public ExceptionRuleCollectionConfigurationElement ExceptionRules
        {
            get 
            {
                return (ExceptionRuleCollectionConfigurationElement)GetChildElement("exceptionRules", typeof(ExceptionRuleCollectionConfigurationElement)); 
            }
        }
    }

    /// <summary>
    /// Represents a Country with it's code in the country collection
    /// </summary>
    public class CountryConfigurationElement : ConfigurationElement
    {
        /// <summary>
        /// Creates a new country instance
        /// </summary>
        public CountryConfigurationElement()
        {
        }

        /// <summary>
        /// The code of the country
        /// </summary>
        public string Code
        {
            get { return (string)base["code"]; }
            set { base["code"] = (string)value; }
        }
    }

    /// <summary>
    /// Represents a collection with country objects
    /// </summary>
    public class CountryCollectionConfigurationElement : ConfigurationElementCollectionBase<CountryConfigurationElement>
    {
        /// <summary>
        /// Creates a new country collection instance
        /// </summary>
        public CountryCollectionConfigurationElement()
        {
        }

        /// <summary>
        /// Adds a new country object to the collection
        /// </summary>
        /// <param name="code">The country code</param>
        /// <returns>The country object</returns>
        public CountryConfigurationElement Add(string code)
        {
            CountryConfigurationElement element = CreateElement();
            element.Code = code;
            return Add(element);
        }

        /// <summary>
        /// Creates a new country element
        /// </summary>
        /// <param name="elementTagName">Not used (the xml tag of the configuration node)</param>
        /// <returns>A new country object</returns>
        protected override CountryConfigurationElement CreateNewElement(string elementTagName)
        {
            return new CountryConfigurationElement();
        }
    }

    /// <summary>
    /// Represents an exception rule that is checked first by the geoblocker
    /// </summary>
    public class ExceptionRuleConfigurationElement : ConfigurationElement
    {
        /// <summary>
        /// Creates a new exception rule instance
        /// </summary>
        public ExceptionRuleConfigurationElement()
        {
        }

        /// <summary>
        /// Creates a new exception rule instance
        /// </summary>
        /// <param name="allowedMode">The mode of the exception rule</param>
        /// <param name="ipAddress">The ip address of the exception rule</param>
        /// <param name="mask">The mask of the exception rule</param>
        /// <param name="family">The family of the exception rule</param>
        public ExceptionRuleConfigurationElement(bool allow, string ipAddress, string mask, string family)
        {
            this.Allow = allow;
            this.Address = ipAddress;
            this.Mask = mask;
            this.Family = family;
        }

        /// <summary>
        /// The mode of the exception rule
        /// </summary>
        public bool Allow
        {
            get { return (bool)base["allow"]; }
            set { base["allow"] = (bool)value; }
        }

        /// <summary>
        /// The ip address of the exception rule
        /// </summary>
        public string Address
        {
            get { return (string)base["address"]; }
            set { base["address"] = (string)value; }
        }

        /// <summary>
        /// The mask of the exception rule
        /// </summary>
        public string Mask
        {
            get { return (string)base["mask"]; }
            set { base["mask"] = (string)value; }
        }

        /// <summary>
        /// The family of the address
        /// </summary>
        public string Family
        {
            get { return (string)base["family"]; }
            set { base["family"] = (string)value; }
        }
    }

    /// <summary>
    /// Represents a collection with exception rule objects
    /// </summary>
    public class ExceptionRuleCollectionConfigurationElement : ConfigurationElementCollectionBase<ExceptionRuleConfigurationElement>
    {
        /// <summary>
        /// Creates a new exception rule collection instance
        /// </summary>
        public ExceptionRuleCollectionConfigurationElement()
        {
        }

        /// <summary>
        /// Adds a new exception rule object to the collection
        /// </summary>
        /// <param name="code">The exception mode of the exception rule</param>
        /// <param name="ipAddress">The ip address of the exception rule</param>
        /// <param name="mask">The subnet mask of the exception rule</param>
        /// <returns>The exception rule object</returns>
        public ExceptionRuleConfigurationElement Add(bool allowedMode, string ipAddress, string mask, string family)
        {
            ExceptionRuleConfigurationElement element = CreateElement();
            element.Allow = allowedMode;
            element.Address = ipAddress;
            element.Mask = mask;
            element.Family = family;
            return Add(element);
        }

        /// <summary>
        /// Creates a new exception rule element
        /// </summary>
        /// <param name="elementTagName">Not used (the xml tag of the configuration node)</param>
        /// <returns>A new exception rule object</returns>
        protected override ExceptionRuleConfigurationElement CreateNewElement(string elementTagName)
        {
            return new ExceptionRuleConfigurationElement();
        }
    }
}
