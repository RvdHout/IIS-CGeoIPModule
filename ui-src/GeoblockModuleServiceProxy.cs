#nullable disable
/* GeoblockModuleServiceProxy.cs
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

using Microsoft.Web.Management.Client;
using Microsoft.Web.Management.Server;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Diagnostics;

namespace CGeoIPModule
{
    /// <summary>
    /// Uses the module service at the server to get and update the geoblock configuration
    /// </summary>
    public class GeoblockModuleServiceProxy : ModuleServiceProxy
    {
#if DEBUG
        private void DbgWrite(string format, params object[] args)
        {
            try
            {
                string str = string.Format(format, args);
                Trace.WriteLine(string.Format("[{0}]: {1}", "CGeoIPModule", str));
            }
            catch (Exception exception)
            {
                Trace.WriteLine(string.Format("DbgWrite::Error: {0}", exception.Message));
            }
        }
#endif
        /// <summary>
        /// Gets the geoblock configuration from the module service
        /// </summary>
        /// <returns>The geoblock configuration</returns>
        public GeoblockConfiguration GetGeoblockConfiguration()
        {
#if DEBUG
            DbgWrite("GetGeoblockConfiguration() start");
#endif
            PropertyBag config = (PropertyBag)Invoke("GetGeoblockConfiguration");

            GeoblockConfiguration result = new GeoblockConfiguration();
#if DEBUG
            DbgWrite("GetGeoblockConfiguration() created GeoblcokConfiguration result");
#endif

            result.Enabled = (bool)config[0];
            result.Action = (string)config[1];
            result.Path = (string)config[2];
            result.AllowedMode = (bool)config[3];
#if DEBUG
            DbgWrite("GetGeoblockConfiguration() assigned values");
#endif

            result.CountryCodes = new List<Country>();
            ArrayList countries = (ArrayList)config[4];
            foreach (PropertyBag item in countries)
            {
                result.CountryCodes.Add(new Country((string)item[0], null));
            }

#if DEBUG
            DbgWrite("GetGeoblockConfiguration() assigned country codes");
#endif
            result.ExceptionRules = new List<ExceptionRule>();
            ArrayList exceptionRules = (ArrayList)config[5];
            foreach (PropertyBag item in exceptionRules)
            {
                result.ExceptionRules.Add(new ExceptionRule((bool)item[0], (string)item[1], (string)item[2], (string)item[3]));
            }

            return result;
        }

        /// <summary>
        /// Updates the geoblock configuration using the module service
        /// </summary>
        /// <param name="updatedGeoblockConfiguration">The new geoblock configuration</param>
        public void UpdateGeoblockConfiguration(GeoblockConfiguration updatedGeoblockConfiguration)
        {
            PropertyBag config = new PropertyBag();
            config.Add(0, updatedGeoblockConfiguration.Enabled);
            config.Add(1, updatedGeoblockConfiguration.Action);
            config.Add(2, updatedGeoblockConfiguration.Path);
            config.Add(3, updatedGeoblockConfiguration.AllowedMode);

            ArrayList countries = new ArrayList();
            foreach (Country country in updatedGeoblockConfiguration.CountryCodes)
            {
                PropertyBag item = new PropertyBag();
                item.Add(0, country.CountryCode);
                countries.Add(item);
            }
            config.Add(4, countries);

            ArrayList exceptionRules = new ArrayList();
            foreach (ExceptionRule exceptionRule in updatedGeoblockConfiguration.ExceptionRules)
            {
                PropertyBag item = new PropertyBag();
                item.Add(0, exceptionRule.Allow);
                item.Add(1, exceptionRule.Address);
                item.Add(2, exceptionRule.Mask);
                item.Add(3, exceptionRule.Family);
                exceptionRules.Add(item);
            }
            config.Add(5, exceptionRules);

            Invoke("UpdateGeoblockConfiguration", config);
        }
    }
}
