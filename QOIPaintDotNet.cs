// QOIPaintDotNet.cs - QOI Paint.NET plugin
//
// Copyright (C) 2021 Piotr Fusik
//
// MIT License:
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

using System;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Reflection;
using System.Runtime.InteropServices;

using PaintDotNet;

[assembly: AssemblyTitle("Paint.NET Quite OK Image (QOI) plugin")]
[assembly: AssemblyCompany("Piotr Fusik")]
[assembly: AssemblyCopyright("Copyright © 2021")]
[assembly: AssemblyVersion("0.0.1.0")]
[assembly: AssemblyFileVersion("0.0.1.0")]

namespace QOI.PaintDotNet
{
	class QOIFileType : FileType
	{
		public QOIFileType() : base("Quite OK Image", new FileTypeOptions { LoadExtensions = new string[] { ".qoi" } })
		{
		}

		protected override Document OnLoad(Stream input)
		{
			// Read
			long longLength = input.Length;
			if (longLength > 0x7fffffc7) // max byte array length according to https://docs.microsoft.com/en-us/dotnet/api/system.array?view=net-6.0
				throw new Exception("File too long");
			int encodedLength = (int) longLength;
			byte[] encoded = new byte[encodedLength];
			encodedLength = input.Read(encoded, 0, encodedLength);

			// Decode
			QOIDecoder qoi = new QOIDecoder();
			if (!qoi.Decode(encoded, encodedLength))
				throw new Exception("Decoding error");
			int width = qoi.GetWidth();

			// Pass to Paint.NET
			GCHandle pinnedPixels = GCHandle.Alloc(qoi.GetPixels(), GCHandleType.Pinned);
			using (Bitmap bitmap = new Bitmap(width, qoi.GetHeight(), width << 2, PixelFormat.Format32bppArgb, pinnedPixels.AddrOfPinnedObject())) {
				pinnedPixels.Free();
				return Document.FromImage(bitmap);
			}
		}
	}

	public class QOIFileTypeFactory : IFileTypeFactory
	{
		public FileType[] GetFileTypeInstances()
		{
			return new FileType[] { new QOIFileType() };
		}
	}
}
