#nullable disable
/* GeoblockConfiguration.cs
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
using System.Xml.Serialization;
using Microsoft.Web.Management.Server;

namespace CGeoIPModule
{
    /// <summary>
    /// Wraps around the ConfigurationSection.
    /// </summary>
    public class GeoblockConfiguration
    {
        /// <summary>
        /// Creates a geoblock configuration instance
        /// </summary>
        public GeoblockConfiguration() { }

        /// <summary>
        /// Specifies whether or not the module is enabled
        /// </summary>
        public bool Enabled { get; set; }

        /// <summary>
        /// Specifies whether or not to use REMOTE_ADDR to obtain client ip
        /// </summary>
        public bool RemoteAddr { get; set; }

        /// <summary>
        /// Specifies the Deny Action Type
        /// </summary>
        public string Action { get; set; }

        /// <summary>
        /// Points to the GeoIP.dat file
        /// </summary>
        public string Path { get; set; }

        /// <summary>
        /// Whether the selected country codes are allowed or not
        /// </summary>
        public bool AllowedMode { get; set; }

        /// <summary>
        /// A collection that holds the selected country codes
        /// </summary>
        public List<Country> CountryCodes { get; set; }

        /// <summary>
        /// A collection that holds the exception rules
        /// </summary>
        public List<ExceptionRule> ExceptionRules { get; set; }
    }

    /// <summary>
    /// Used to hold a country item
    /// </summary>
    public class Country
    {
        /// <summary>
        /// The country code of this item
        /// </summary>
        public string CountryCode { get; set; }

        /// <summary>
        /// The country name corresponding to the country code
        /// </summary>
        public string CountryName { get; set; }

        /// <summary>
        /// Creates a new Country
        /// </summary>
        /// <param name="countryCode">The country code</param>
        /// <param name="country">The corresponding country name</param>
        public Country(string countryCode, string country)
        {
            this.CountryCode = countryCode;
            this.CountryName = country;
        }

        /// <summary>
        /// Returns the textual representation of the country
        /// </summary>
        /// <returns>The textual representation of the country</returns>
        public override string ToString()
        {
            return CountryName + " (" + CountryCode + ")";
        }
    }

    /// <summary>
    /// Used to hold an exception rule item.
    /// </summary>
    public class ExceptionRule
    {
        /// <summary>
        /// The mode of this exception rule
        /// </summary>
        public bool Allow { get; set; }

        /// <summary>
        /// The ip address of this exception rule
        /// </summary>
        public string Address { get; set; }

        /// <summary>
        /// The mask of this exception rule
        /// </summary>
        public string Mask { get; set; }

        /// <summary>
        /// The family of this exception rule
        /// </summary>
        public string Family { get; set; }

        /// <summary>
        /// Creates a new exception rule
        /// </summary>
        /// <param name="allowedMode">The mode of the exception rule</param>
        /// <param name="ipAddress">The ip address of the exception rule</param>
        /// <param name="mask">The mask of the exception rule</param>
        public ExceptionRule(bool allowedMode, string address, string mask, string family)
        {
            this.Allow = allowedMode;
            this.Address = address;
            this.Mask = mask;
            this.Family = family;
        }

        /// <summary>
        /// The textual representation of the alowed mode
        /// </summary>
        public string Mode
        {
            get
            {
                if (this.Allow)
                    return "Allow";
                else
                    return "Deny";
            }
        }

        /// <summary>
        /// The textual representation of the ip address and subnet mask
        /// </summary>
        public string Requestor
        {
            get
            {
                string result = this.Address;
                if (!String.IsNullOrEmpty(this.Mask))
                {
                    result += " (" + this.Mask + ")";
                }
                return result;
            }
        }

        /// <summary>
        /// Returns the textual representation of the exception rule
        /// </summary>
        /// <returns>The textual representation of the exception rule</returns>
        public override string ToString()
        {
            return this.Mode + " " + this.Requestor;
        }
    }
}
