﻿#nullable disable
/* GeoblockModuleService.cs
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
using Microsoft.Web.Management.Server;
using System.Collections;
using System.Diagnostics;

namespace CGeoIPModule
{
    /// <summary>
    /// Module service to enable the client to get the settings
    /// </summary>
    public class GeoblockModuleService : ModuleService
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
        /// Gets the configuration of the geoblock module
        /// </summary>
        /// <returns>The geoblock configuration in a propertybag</returns>
        [ModuleServiceMethod(PassThrough = true)]
        public PropertyBag GetGeoblockConfiguration()
        {
#if DEBUG
            DbgWrite("GetGeoblockConfiguration() start");
#endif
            GeoblockConfigurationSection config = (GeoblockConfigurationSection)ManagementUnit.Configuration.GetSection(GeoblockConfigurationSection.SectionName, typeof(GeoblockConfigurationSection));

            PropertyBag result = new PropertyBag();
            result.Add(0, config.Enabled);
            result.Add(1, config.RemoteAddr);
            result.Add(2, config.Action);
            result.Add(3, config.Path);
            result.Add(4, config.Allow);


#if DEBUG
            DbgWrite("GetGeoblockConfiguration() adding countrycodes");
#endif
            ArrayList countries = new ArrayList();
            foreach (CountryConfigurationElement country in config.CountryCodes)
            {
                PropertyBag item = new PropertyBag();
                item.Add(0, country.Code);
                countries.Add(item);
            }
            result.Add(5, countries);

#if DEBUG
            DbgWrite("GetGeoblockConfiguration() adding exception rules");
#endif
            ArrayList exceptionRules = new ArrayList();
            foreach (ExceptionRuleConfigurationElement exceptionRule in config.ExceptionRules)
            {
#if DEBUG
                DbgWrite(string.Format("GetGeoblockConfiguration() adding exception rule {0} {1} {2} {3}", exceptionRule.Allow, exceptionRule.Address, exceptionRule.Mask, exceptionRule.Family));
#endif
                PropertyBag item = new PropertyBag();
                item.Add(0, exceptionRule.Allow);
                item.Add(1, exceptionRule.Address);
                item.Add(2, exceptionRule.Mask);
                item.Add(3, exceptionRule.Family);
                exceptionRules.Add(item);
            }
            result.Add(6, exceptionRules);
#if DEBUG
            DbgWrite("GetGeoblockConfiguration() end");
#endif

            return result;
        }

        /// <summary>
        /// Writes the configuration of the geoblock module
        /// </summary>
        /// <param name="updatedGeoblockConfiguration">The new geoblock configuration in a propertybag</param>
        [ModuleServiceMethod(PassThrough = true)]
        public void UpdateGeoblockConfiguration(PropertyBag updatedGeoblockConfiguration)
        {
            if (updatedGeoblockConfiguration == null)
            {
                throw new ArgumentNullException("updatedGeoblockConfiguration");
            }

            GeoblockConfigurationSection config = (GeoblockConfigurationSection)ManagementUnit.Configuration.GetSection(GeoblockConfigurationSection.SectionName, typeof(GeoblockConfigurationSection));

            config.Enabled = (bool)updatedGeoblockConfiguration[0];
            config.RemoteAddr = (bool)updatedGeoblockConfiguration[1];
            config.Action = (string)updatedGeoblockConfiguration[2];
            config.Path = (string)updatedGeoblockConfiguration[3];
            config.Allow = (bool)updatedGeoblockConfiguration[4];
            config.CountryCodes.Clear();
            ArrayList countries = (ArrayList)updatedGeoblockConfiguration[5];
            foreach (PropertyBag item in countries)
            {
                config.CountryCodes.Add((string)item[0]);
            }
            config.ExceptionRules.Clear();
            ArrayList exceptionRules = (ArrayList)updatedGeoblockConfiguration[6];
            foreach (PropertyBag item in exceptionRules)
            {
                config.ExceptionRules.Add((bool)item[0], (string)item[1], (string)item[2], (string)item[3]);
            }
            
            ManagementUnit.Update();
        }
    }
}
