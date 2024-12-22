#nullable disable
/* AddExceptionRuleForm.cs
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
using System.ComponentModel;
using System.Linq;
using System.Net;
using System.Windows.Forms;
using NetTools;

namespace CGeoIPModule
{
    /// <summary>
    /// The dialog form to specify an exception rule for the geoblocker
    /// </summary>
    public partial class AddExceptionRuleForm : Form
    {
        private bool allowMode;
        /// <summary>
        /// Specifies whether to add an allow or deny exception rule.
        /// </summary>
        public bool AllowMode
        {
            get { return allowMode; }
        }

        private BindingList<ExceptionRule> existingRules;
        /// <summary>
        /// The existing rules to check for conflicts.
        /// </summary>
        public BindingList<ExceptionRule> ExistingRules
        {
            get { return existingRules; }
        }

        private ExceptionRule exceptionRule;
        /// <summary>
        /// Contains the exception rule if the user specified correct input and presses OK
        /// </summary>
        public ExceptionRule ExceptionRule 
        {
            get { return exceptionRule; }
        }

        /// <summary>
        /// Creates a new form
        /// </summary>
        /// <param name="allowMode">Whether to add an allow or deny exception rule</param>
        public AddExceptionRuleForm(bool allowMode, BindingList<ExceptionRule> existingRules)
        {
            this.allowMode = allowMode;
            this.existingRules = existingRules;
            InitializeComponent();
        }

        /// <summary>
        /// Sets the correct texts based on the allowmode
        /// </summary>
        /// <param name="sender">Not used</param>
        /// <param name="e">Not used</param>
        private void AddExceptionRuleForm_Load(object sender, EventArgs e)
        {
            if (AllowMode)
            {
                this.Text = "Add Allow Exception Rule";
                LCaption.Text = "Allow access for the following address or range:";
            }
            else
            {
                this.Text = "Add Deny Exception Rule";
                LCaption.Text = "Deny access for the following address or range:";
            }
        }

        /// <summary>
        /// Sets focus on the textbox
        /// </summary>
        /// <param name="sender">Not used</param>
        /// <param name="e">Not used</param>
        private void AddExceptionRuleForm_Shown(object sender, EventArgs e)
        {
            TBSpecificIpAddress.Focus();
        }

        /// <summary>
        /// Fired when the user clicks the OK button. Checks if the user input is valid. If so, it sets the exception rule and closes the form
        /// </summary>
        /// <param name="sender">Not used</param>
        /// <param name="e">Not used</param>
        private void BOk_Click(object sender, EventArgs e)
        {
            string family = "ipv4";

            if (ipv4Checkbox.Checked)
            {
                family = "ipv4";
            }
            if (ipv6Checkbox.Checked)
            {
                family = "ipv6";
            }

            if (RBSpecificIpAddress.Checked)
            {
                IPAddress ip;
                string mask = "255.255.255.255";
                if(family == "ipv6")
                    mask = "128";

                if (!IPAddress.TryParse(TBSpecificIpAddress.Text, out ip))
                {
                    MessageBox.Show(this, "'" + TBSpecificIpAddress.Text + "' is an invalid IP address.", this.Text, MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                    return;
                }
                else
                {
                    //We will check against existing exception rules if there is a conflict or overlap
                    ExceptionRule newRule = new ExceptionRule(this.AllowMode, ip.ToString(), mask, family);
                    if (CheckNewRule(newRule))
                    {
                        this.exceptionRule = newRule;
                        this.DialogResult = DialogResult.OK;
                        this.Close();
                    }
                }
            }
            else
            {
                IPAddress range;
                IPAddress mask;
                if (!IPAddress.TryParse(TBAddressRange.Text, out range))
                {
                    MessageBox.Show(this, "'" + TBAddressRange.Text + "' is an invalid IP address.", this.Text, MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                    return;
                }

                if (!IPAddress.TryParse(TBMask.Text, out mask))
                {
                    MessageBox.Show(this, "'" + TBMask.Text + "' is an invalid subnet mask.", this.Text, MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                    return;
                }

                try
                {
                    var cidr = IPAddressRange.Parse(TBAddressRange.Text + "/" + TBMask.Text).ToCidrString();
                }
                catch 
                {
                    MessageBox.Show(this, "'" + TBMask.Text + "' is an invalid subnet mask.", this.Text, MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
                    return;
                }

                //We will check against existing exception rules if there is a conflict or overlap
                ExceptionRule newRule = new ExceptionRule(this.AllowMode, range.ToString(), mask.ToString(), family.ToString());
                if (CheckNewRule(newRule))
                {
                    this.exceptionRule = newRule;
                    this.DialogResult = DialogResult.OK;
                    this.Close();
                }
            }
        }

        /// <summary>
        /// Fires when the radio button changes state. Sets the enabled state of the corresponding textbox.
        /// </summary>
        /// <param name="sender">Not used</param>
        /// <param name="e">Not used</param>
        private void RBSpecificIpAddress_CheckedChanged(object sender, EventArgs e)
        {
            if (RBSpecificIpAddress.Checked)
            {
                TBSpecificIpAddress.Enabled = true;
            }
            else
            {
                TBSpecificIpAddress.Enabled = false;
            }
        }

        /// <summary>
        /// Fires when the radio button changes state. Sets the enabled state of the corresponding textboxes.
        /// </summary>
        /// <param name="sender">Not used</param>
        /// <param name="e">Not used</param>
        private void RBAddressRange_CheckedChanged(object sender, EventArgs e)
        {
            if (RBAddressRange.Checked)
            {
                TBAddressRange.Enabled = true;
                TBMask.Enabled = true;
            }
            else
            {
                TBAddressRange.Enabled = false;
                TBMask.Enabled = false;
            }
            SetOkButtonStatus(sender, e);
        }

        /// <summary>
        /// Fires when a value of one of the textboxes changes. Sets the enabled status of the OK button
        /// </summary>
        /// <param name="sender">Not used</param>
        /// <param name="e">Not used</param>
        private void SetOkButtonStatus(object sender, EventArgs e)
        {
            if (RBSpecificIpAddress.Checked)
            {
                if (TBSpecificIpAddress.Text != String.Empty)
                    BOk.Enabled = true;
                else
                    BOk.Enabled = false;
            }
            if (RBAddressRange.Checked)
            {
                if (TBAddressRange.Text != String.Empty && TBMask.Text != String.Empty)
                    BOk.Enabled = true;
                else
                    BOk.Enabled = false;
            }
        }

        /// <summary>
        /// Checks for conflicts between the new rule and the existing exception rules
        /// </summary>
        /// <param name="newRule">The new rule to check</param>
        /// <returns>True if there is no conflict. False otherwise</returns>
        private bool CheckNewRule(ExceptionRule newRule)
        {
            foreach (ExceptionRule existingRule in this.existingRules)
            {
                if (String.IsNullOrEmpty(newRule.Mask))
                {
                    if (String.IsNullOrEmpty(existingRule.Mask))
                    {
                        if (newRule.Address.Equals(existingRule.Address))
                        {
                            if (newRule.Allow == existingRule.Allow)
                            {
                                MessageBox.Show(this, "This exception rule allready exists", this.Text, MessageBoxButtons.OK, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1);
                                return false;
                            }
                            else
                            {
                                MessageBox.Show(this, "This exception rule conflicts with rule: " + existingRule.ToString(), this.Text, MessageBoxButtons.OK, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1);
                                return false;
                            }
                        }
                    }
                    else
                    {
                        if (IPUtilities.IsInSameSubnet(newRule.Address, existingRule.Address, existingRule.Mask))
                        {
                            if (newRule.Allow == existingRule.Allow)
                            {
                                DialogResult result = MessageBox.Show(this, "This exception rule overlaps with rule: " + existingRule.ToString() + ". Add it anyway?", this.Text, MessageBoxButtons.YesNoCancel, MessageBoxIcon.Warning, MessageBoxDefaultButton.Button1);
                                if (result == DialogResult.Yes)
                                {
                                    return true;
                                }
                                else
                                {
                                    return false;
                                }
                            }
                            else
                            {
                                MessageBox.Show(this, "This exception rule conflicts with rule: " + existingRule.ToString(), this.Text, MessageBoxButtons.OK, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1);
                                return false;
                            }
                        }
                    }
                }
                else
                {
                    if (String.IsNullOrEmpty(existingRule.Mask))
                    {
                        if (IPUtilities.IsInSameSubnet(existingRule.Address, newRule.Address, newRule.Mask))
                        {
                            if (newRule.Allow == existingRule.Allow)
                            {
                                DialogResult result = MessageBox.Show(this, "This exception rule overlaps with rule: " + existingRule.ToString() + ". Add it anyway?", this.Text, MessageBoxButtons.YesNoCancel, MessageBoxIcon.Warning, MessageBoxDefaultButton.Button1);
                                if (result == DialogResult.Yes)
                                {
                                    return true;
                                }
                                else
                                {
                                    return false;
                                }
                            }
                            else
                            {
                                MessageBox.Show(this, "This exception rule conflicts with rule: " + existingRule.ToString(), this.Text, MessageBoxButtons.OK, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1);
                                return false;
                            }
                        }
                    }
                    else
                    {
                        if (IPUtilities.IsInSameSubnet(newRule.Address, existingRule.Address, existingRule.Mask) || IPUtilities.IsInSameSubnet(existingRule.Address, newRule.Address, newRule.Mask))
                        {
                            if (newRule.Allow == existingRule.Allow)
                            {
                                DialogResult result = MessageBox.Show(this, "This exception rule overlaps with rule: " + existingRule.ToString() + ". Add it anyway?", this.Text, MessageBoxButtons.YesNoCancel, MessageBoxIcon.Warning, MessageBoxDefaultButton.Button1);
                                if (result == DialogResult.Yes)
                                {
                                    return true;
                                }
                                else
                                {
                                    return false;
                                }
                            }
                            else
                            {
                                MessageBox.Show(this, "This exception rule conflicts with rule: " + existingRule.ToString(), this.Text, MessageBoxButtons.OK, MessageBoxIcon.Error, MessageBoxDefaultButton.Button1);
                                return false;
                            }
                        }
                    }
                }
            }
            return true;
        }

        private void checkBox1_CheckedChanged(object sender, EventArgs e)
        {
            if (ipv4Checkbox.Checked)
            {
                ipv6Checkbox.Checked = false;
            }
        }

        private void checkBox2_CheckedChanged(object sender, EventArgs e)
        {
            if (ipv6Checkbox.Checked)
            {
                ipv4Checkbox.Checked = false;
            }
        }
    }
}
