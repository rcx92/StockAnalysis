using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;

namespace MergeData
{
    class Program
    {

        static void copy(string newDataPath, string matchFolder)
        {
            FileStream fs = new FileStream(newDataPath, FileMode.Open, FileAccess.Read);
            StreamReader sr = new StreamReader(fs);
            sr.ReadLine();//format行
            while (!sr.EndOfStream)
            {
                string data = sr.ReadLine();
                string[] nodes = data.Split(',');
                string fileName = matchFolder + @"\" + nodes[0] + ".csv";
                FileStream fsw = new FileStream(fileName, FileMode.Append, FileAccess.Write);
                StreamWriter sw = new StreamWriter(fsw);
                sw.WriteLine(data);
                sw.Close();
                fsw.Close();
            }
            sr.Close();
            fs.Close();
            File.Delete(newDataPath);
        }
        static void Main(string[] args)
        {
            string path = System.Environment.CurrentDirectory;
            string newDataPath = path + @"\new data";
            string stockDataPath = path + @"\stock data";
            string indexDataPath = path + @"\index data";
            var data = Directory.GetFiles(newDataPath);
            foreach (var x in data)
            {
                string tmp = x.ToLower();
                if (!tmp.Contains(".csv")) continue;
                if (tmp.Contains("index"))
                {
                    copy(x, indexDataPath);
                }
                else
                {
                    copy(x, stockDataPath);
                }
            }
            Console.ReadLine();
        }
    }
}
