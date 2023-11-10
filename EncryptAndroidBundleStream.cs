using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Threading;
using DH.Foundations.FileSystem;
using Unity.Collections;
using Unity.Collections.LowLevel.Unsafe;
using UnityEngine;

namespace DH.Asset
{
#if UNITY_ANDROID && !UNITY_EDITOR
    public class EncryptAndroidBundleStream : Stream
    {
        private const string LibName = "libnativelib";

        [DllImport(LibName)]
        public static extern unsafe void* OpenFile(string fileName);
        
        [DllImport(LibName)]
        public static extern unsafe void CloseFile(void* assetPtr);
        
        [DllImport(LibName)]
        public static extern unsafe long GetLength(void* assetPtr);
        
        [DllImport(LibName)]
        public static extern unsafe long GetPosition(void* assetPtr);
        
        [DllImport(LibName)]
        public static extern unsafe long Seek(void* assetPtr,long offset,int whence);
        
        [DllImport(LibName)]
        public static extern unsafe int Read(void* assetPtr,void*buffPtr,int size);
        
        private long position;
        private readonly int pwdLength;
        private unsafe void* assetPtr;
        private readonly int unityThreadId;
        private static bool initAssetManager;

        private static int? basePathLength;

        public EncryptAndroidBundleStream(string fullPath, FileSystemAccess access, bool createNew)
        {
            if (basePathLength == null)
            {
                basePathLength = Application.streamingAssetsPath.Length + 1;
            }
            position = 0;
            pwdLength = EncryptBundleStream.password.Length;
            unsafe
            {
                assetPtr = OpenFile(fullPath.Substring(basePathLength.Value));
            }
            unityThreadId = Thread.CurrentThread.ManagedThreadId;
        }

        public override void Close()
        {
            unsafe
            {
                CloseFile(assetPtr);
            }
            base.Close();
        }

        public override long Seek(long offset, SeekOrigin origin)
        {
            position = offset;
            try
            {
                if (!RunInMainThread)
                {
                    AndroidJNI.AttachCurrentThread();
                }

                unsafe
                {
                    return Seek(assetPtr, offset, (int)origin);
                }
            }
            finally
            {
                if (!RunInMainThread)
                {
                    AndroidJNI.DetachCurrentThread();
                }
            }
        }

        public override void Flush()
        {
            throw new NotImplementedException();
        }

        public override int Read(byte[] array, int offset, int count)
        {
            long pwdIndex = position;
            int readCount = 0; 
            try
            {
                unsafe
                {
                    if (!RunInMainThread)
                    {
                        AndroidJNI.AttachCurrentThread();
                    }

                    GCHandle gcHandle = GCHandle.Alloc(array, GCHandleType.Pinned);
                    readCount = Read(assetPtr, (void*) ((IntPtr) (void*) gcHandle.AddrOfPinnedObject() + offset * UnsafeUtility.SizeOf<byte>()), count);
                    gcHandle.Free();
                }
            }
            finally
            {
                if (!RunInMainThread)
                {
                    AndroidJNI.DetachCurrentThread();
                }
            }
            for (int i = offset; i < count && i < readCount; i++)
            {
                array[i] ^= EncryptBundleStream.password[(pwdIndex + i) % pwdLength];
            }
            position += readCount;
            return readCount;
        }

        public override void Write(byte[] array, int offset, int count)
        {
            throw new Exception("Not support");
        }

        public override bool CanRead => true;
        public override bool CanSeek => true;
        public override bool CanWrite => false;

        public override long Length
        {
            get
            {
                try
                {
                    if (!RunInMainThread)
                    {
                        AndroidJNI.AttachCurrentThread();
                    }

                    unsafe
                    {
                        return GetLength(assetPtr);
                    }
                }
                finally
                {
                    if (!RunInMainThread)
                    {
                        AndroidJNI.DetachCurrentThread();
                    }
                }
            }
        }

        public override long Position
        {
            get
            {
                try
                {
                    if (!RunInMainThread)
                    {
                        AndroidJNI.AttachCurrentThread();
                    }

                    unsafe
                    {
                        return GetPosition(assetPtr);
                    }
                }
                finally
                {
                    if (!RunInMainThread)
                    {
                        AndroidJNI.DetachCurrentThread();
                    }
                }
            }
            set
            {
                try
                {
                    if (!RunInMainThread)
                    {
                        AndroidJNI.AttachCurrentThread();
                    }

                    unsafe
                    {
                        Seek(assetPtr, value, 0);
                    }
                }
                finally
                {
                    if (!RunInMainThread)
                    {
                        AndroidJNI.DetachCurrentThread();
                    }
                }
            }
        }

        public override void SetLength(long value)
        {
            throw new System.NotImplementedException();
        }

        private bool RunInMainThread => Thread.CurrentThread.ManagedThreadId == unityThreadId;
    }
#endif
}