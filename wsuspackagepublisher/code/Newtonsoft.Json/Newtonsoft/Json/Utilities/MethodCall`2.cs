﻿// Decompiled with JetBrains decompiler
// Type: Newtonsoft.Json.Utilities.MethodCall`2
// Assembly: Newtonsoft.Json, Version=12.0.0.0, Culture=neutral, PublicKeyToken=30ad4fe6b2a6aeed
// MVID: 2676A2DA-6EDC-420E-890E-D28AA4572EE5
// Assembly location: C:\Users\Administrator.THEFLIGHTSIMS\Downloads\Release.v1.4.2203.19\Newtonsoft.Json.dll


#nullable enable
namespace Newtonsoft.Json.Utilities
{
  internal delegate TResult MethodCall<T, TResult>(T target, params object?[] args);
}