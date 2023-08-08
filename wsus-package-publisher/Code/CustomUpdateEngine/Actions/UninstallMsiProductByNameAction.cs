﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.RegularExpressions;
using System.Xml;
using Microsoft.Win32;
using WindowsInstaller;

namespace CustomUpdateEngine
{
    public class UninstallMsiProductByNameAction : GenericAction
    {
        public struct MsiProduct
        {
            private string _name;
            private string _id;

            public MsiProduct(string name, string id)
            {
                _name = name;
                _id = id;
            }

            public string Name { get { return _name; } private set { _name = value; } }
            public string ID { get { return _id; } private set { _id = value; } }
        }

        public UninstallMsiProductByNameAction(string xmlFragment)
        {
            Logger.Write("Get UninstallMsiProductByNameAction from : " + xmlFragment);

            XmlReader reader = XmlReader.Create(new System.IO.StringReader(xmlFragment));

            if (!reader.ReadToFollowing("ApplicationName"))
                throw new ArgumentException("Unable to find token : ApplicationName");
            ApplicationName = reader.ReadString();
            if (!reader.ReadToFollowing("Exceptions"))
                throw new ArgumentException("Unable to find token : Exceptions");
            Exceptions = reader.ReadString();
            if (!reader.ReadToFollowing("Parameters"))
                throw new ArgumentException("Unable to find token : Parameters");
            Parameters = reader.ReadString();
            if (!reader.ReadToFollowing("DontUninstallIfNoException"))
                throw new ArgumentException("Unable to find token : DontUninstallIfNoException");
            DontUninstallIfNoException = Convert.ToBoolean(reader.ReadString());
            if (!reader.ReadToFollowing("KillProcess"))
                throw new ArgumentException("Unable to find token : KillProcess");
            KillProcess = Convert.ToBoolean(reader.ReadString());
            if (!reader.ReadToFollowing("KillAfter"))
                throw new ArgumentException("Unable to find token : KillAfter");
            KillAfter = reader.ReadElementContentAsInt();

            Logger.Write("End of Initializing of UninstallMsiProductByNameAction.");
        }

        public string ApplicationName { get; set; }
        public string Exceptions { get; set; }
        public string Parameters { get; set; }
        public bool DontUninstallIfNoException { get; set; }
        public bool KillProcess { get; set; }
        public int KillAfter { get; set; }

        public override void Run(ref ReturnCodeAction returnCode)
        {
            Logger.Write("Running UninstallMsiProductByNameAction. ApplicationName= " + this.ApplicationName + " Exceptions= " + this.Exceptions + " Parameters= " + this.Parameters);

            try
            {
                Logger.Write("Getting all installed product on this computer");
                List<MsiProduct> installedProducts = GetMsiProducts();
                Logger.Write("Found " + installedProducts.Count + " products installed.");

                if (!this.DontUninstallIfNoException || this.IsAtLeastOneExceptionIsInstalled(installedProducts))
                {
                    Logger.Write("Searching products to uninstall");
                    List<MsiProduct> productsToUninstall = this.GetProductsToUninstall(installedProducts);
                    Logger.Write(productsToUninstall.Count + " products to uninstall.");

                    foreach (MsiProduct product in productsToUninstall)
                    {
                        this.UninstallProduct(product);
                    }
                }
            }
            catch (Exception ex)
            {
                Logger.Write("An error occurs while preparing uninstallation : " + ex.Message);
                throw;
            }

            Logger.Write("End of UninstallMsiProductByNameAction.");
        }

        public static List<MsiProduct> GetMsiProducts()
        {
            List<MsiProduct> msiProducts = new List<MsiProduct>();

            try
            {
                Type type = Type.GetTypeFromProgID("WindowsInstaller.Installer");
                Installer installer = Activator.CreateInstance(type) as Installer;

                StringList products = installer.Products;
                foreach (string productGuid in products)
                {
                    try
                    {
                        msiProducts.Add(new MsiProduct(installer.ProductInfo[productGuid, "ProductName"], GetFormattedIdentifyingNumber(productGuid)));
                    }
                    catch (Exception) { }
                }
            }
            catch (Exception ex)
            {
                Logger.Write("An error occurs while getting list of all MSI products installed on thhis computer : \r\n" + ex.Message);
            }

            return msiProducts;
        }

        private bool IsAtLeastOneExceptionIsInstalled(List<MsiProduct> installedProducts)
        {
            Logger.Write("Searching at least one exception in installed products");
            List<string> exceptions = this.SplitApplicationNames(Exceptions);

            foreach (MsiProduct installedProduct in installedProducts)
            {
                foreach (string exception in exceptions)
                {
                    if (PatternMatchApplicationName(installedProduct.Name, exception))
                    {
                        Logger.Write("At least one exception is installed : " + installedProduct.Name + " (" + installedProduct.ID + ") match exception : " + exception);
                        return true;
                    }
                }
            }
            Logger.Write("No installed products are matching exception");
            return false;
        }

        private static string GetFormattedIdentifyingNumber(string identifyingNumber)
        {
            return identifyingNumber.Substring(1, 36); // Remove leading '{' and trailing '}'
        }

        private List<MsiProduct> GetProductsToUninstall(List<MsiProduct> allInstalledProducts)
        {
            List<MsiProduct> productsToUninstall = new List<MsiProduct>();
            List<string> productToFind = this.SplitApplicationNames(ApplicationName);
            List<string> exceptions = this.SplitApplicationNames(Exceptions);

            foreach (MsiProduct installedProduct in allInstalledProducts)
            {
                foreach (string product in productToFind)
                {
                    if (PatternMatchApplicationName(installedProduct.Name, product))
                    {
                        bool uninstallIt = true;
                        foreach (string exception in exceptions)
                        {
                            if (PatternMatchApplicationName(installedProduct.Name, exception))
                            {
                                uninstallIt = false;
                                Logger.Write(installedProduct.Name + " match exception " + exception + " (it won't be uninstalled)");
                                break;
                            }
                        }
                        if (uninstallIt)
                        {
                            productsToUninstall.Add(installedProduct);
                            Logger.Write(installedProduct.Name + " is selected for uninstallation (" + installedProduct.ID + ")");
                            break;
                        }
                    }
                }
            }

            return productsToUninstall;
        }

        private List<string> SplitApplicationNames(string textToSplit)
        {
            List<string> applicationNames = new List<string>();

            string[] applicationsArray = textToSplit.Split(new char[] { ';' }, StringSplitOptions.RemoveEmptyEntries);
            foreach (string application in applicationsArray)
            {
                applicationNames.Add(application);
            }

            return applicationNames;
        }

        private bool PatternMatchApplicationName(string applicationName, string pattern)
        {
            bool result = false;

            if (pattern.Contains("%") || pattern.Contains("_"))
            {
                Regex regExpr = new Regex("^" + GetRegExpPattern(pattern) + "$", RegexOptions.IgnoreCase);
                if (regExpr.IsMatch(applicationName))
                {
                    result = true;
                }
            }
            else
            {
                Logger.Write("pattern doesn't contains joker characters.");
                result = String.Compare(applicationName, pattern, true) == 0;
            }

            if (result)
                Logger.Write(applicationName + " match pattern " + pattern);
            else
                Logger.Write(applicationName + " don't match pattern " + pattern);

            return result;
        }

        private string GetRegExpPattern(string pattern)
        {
            return pattern.Replace("%", ".*").Replace("_", ".");
        }

        private void UninstallProduct(MsiProduct productToUninstall)
        {
            Logger.Write("Starting uninstallation of : " + productToUninstall.Name);
            try
            {
                System.Diagnostics.Process proc = new System.Diagnostics.Process();
                proc.StartInfo.Arguments = "/X{" + productToUninstall.ID + "} /qn /norestart /log \"C:\\Windows\\Temp\\Uninstall-" + productToUninstall.ID + ".log\"" + (String.IsNullOrWhiteSpace(this.Parameters)?String.Empty:" " + this.Parameters);
                proc.StartInfo.FileName = Tools.GetExpandedPath(@"%windir%\system32\msiexec.exe");

                proc.Start();

                try
                {
                    if (this.KillProcess)
                    {
                        if (!proc.WaitForExit(this.KillAfter * 60 * 1000))
                        {
                            proc.Kill();
                            Logger.Write("Process killed.");
                        }
                    }
                    else
                        proc.WaitForExit(int.MaxValue);

                    Logger.Write("Exiting process. With Exite code : " + proc.ExitCode.ToString());
                }
                catch (Exception)
                {
                    Logger.Write("The process is already stopped or doesn't have start.");
                }

                switch (proc.ExitCode)
                {
                    case 0:
                        Logger.Write("Successfully uninstalled " + productToUninstall.Name);
                        break;
                    case 3010:
                        Logger.Write("Successfully uninstalled " + productToUninstall.Name + " (A restart is required)");
                        break;
                    default:
                        Logger.Write("An error occurs while uninstalling " + productToUninstall.Name + " (MsiError : " + proc.ExitCode + ")");
                        break;
                }
                System.Threading.Thread.Sleep(2000);
            }
            catch (Exception ex)
            {
                Logger.Write("An error occurs while uninstalling " + productToUninstall.Name + "\r\n" + ex.Message);
            }
        }
    }
}
