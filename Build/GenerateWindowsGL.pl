#!/bin/perl -w

use strict;

my %funcs;

while (<DATA>)
{
  chomp;
  next unless m/C3861/;
  
  s/.* error C3861: '//;
  s/'.*//;
  
  $funcs{$_} = 1;
}

open (HEADER, '>', "../Render/WindowsGL.h") or die $!;

print HEADER "// -*- mode: c++; tab-width: 4; c-basic-offset: 4; -*-\n";
print HEADER "#pragma once\n\n\n\n";

foreach my $func (sort keys %funcs)
{
  (my $ucFunc = $func) =~ tr/a-z/A-Z/;
  print HEADER "extern PFN$ucFunc"."PROC $func;\n";
}

print HEADER "\n\n\nvoid WindowsGLInit();\n";

close HEADER;

open (SOURCE, '>', "../Render/WindowsGL.cpp") or die $!;

print SOURCE "// -*- mode: c++; tab-width: 4; c-basic-offset: 4; -*-\n";
print SOURCE "#include \"Render/GL.h\"\n";


foreach my $func (sort keys %funcs)
{
  (my $ucFunc = $func) =~ tr/a-z/A-Z/;
  print SOURCE "PFN$ucFunc"."PROC $func;\n";
}

print SOURCE "\n\n\nvoid WindowsGLInit()\n";
print SOURCE "{\n";

foreach my $func (sort keys %funcs)
{
  (my $ucFunc = $func) =~ tr/a-z/A-Z/;
  my $signature = "PFN$ucFunc"."PROC";
  print SOURCE "    $func = ($signature) wglGetProcAddress(\"$func\");\n";
}
print SOURCE "}\n";

close (SOURCE);


__DATA__
1>------ Build started: Project: 2dVolumetricLighting, Configuration: Debug x64 ------
1>  Texture.cpp
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\texture.cpp(39): error C3861: 'glActiveTexture': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\texture.cpp(57): error C3861: 'glGenFramebuffers': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\texture.cpp(58): error C3861: 'glBindFramebuffer': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\texture.cpp(59): error C3861: 'glFramebufferTexture2D': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\texture.cpp(60): error C3861: 'glBindFramebuffer': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\texture.cpp(79): error C3861: 'glActiveTexture': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\texture.cpp(138): error C3861: 'glDeleteFramebuffers': identifier not found
1>  Shader.cpp
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\shader.cpp(28): warning C4996: 'sscanf': This function or variable may be unsafe. Consider using sscanf_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.
1>  c:\program files (x86)\windows kits\10\include\10.0.10240.0\ucrt\stdio.h(2254): note: see declaration of 'sscanf'
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\shader.cpp(69): error C3861: 'glCreateProgram': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\shader.cpp(72): error C3861: 'glBindAttribLocation': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\shader.cpp(73): error C3861: 'glBindAttribLocation': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\shader.cpp(74): error C3861: 'glBindAttribLocation': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\shader.cpp(75): error C3861: 'glBindAttribLocation': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\shader.cpp(88): error C3861: 'glCreateShader': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\shader.cpp(89): error C3861: 'glShaderSource': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\shader.cpp(90): error C3861: 'glCompileShader': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\shader.cpp(93): error C3861: 'glGetShaderiv': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\shader.cpp(97): error C3861: 'glGetShaderiv': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\shader.cpp(101): error C3861: 'glGetShaderInfoLog': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\shader.cpp(126): error C3861: 'glCreateShader': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\shader.cpp(127): error C3861: 'glShaderSource': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\shader.cpp(128): error C3861: 'glCompileShader': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\shader.cpp(130): error C3861: 'glGetShaderiv': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\shader.cpp(136): error C3861: 'glGetShaderiv': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\shader.cpp(140): error C3861: 'glGetShaderInfoLog': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\shader.cpp(155): error C3861: 'glAttachShader': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\shader.cpp(156): error C3861: 'glAttachShader': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\shader.cpp(160): error C3861: 'glLinkProgram': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\shader.cpp(161): error C3861: 'glGetProgramiv': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\shader.cpp(165): error C3861: 'glGetShaderiv': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\shader.cpp(169): error C3861: 'glGetProgramInfoLog': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\shader.cpp(181): error C3861: 'glValidateProgram': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\shader.cpp(194): error C3861: 'glGetUniformBlockIndex': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\shader.cpp(195): error C3861: 'glGetUniformBlockIndex': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\shader.cpp(196): error C3861: 'glGetUniformBlockIndex': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\shader.cpp(86): warning C4996: 'sprintf': This function or variable may be unsafe. Consider using sprintf_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.
1>  c:\program files (x86)\windows kits\10\include\10.0.10240.0\ucrt\stdio.h(1769): note: see declaration of 'sprintf'
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\shader.cpp(124): warning C4996: 'sprintf': This function or variable may be unsafe. Consider using sprintf_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.
1>  c:\program files (x86)\windows kits\10\include\10.0.10240.0\ucrt\stdio.h(1769): note: see declaration of 'sprintf'
1>  Render.cpp
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(236): error C3861: 'glGenVertexArrays': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(237): error C3861: 'glBindVertexArray': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(250): error C3861: 'glGenBuffers': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(251): error C3861: 'glBindBuffer': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(252): error C3861: 'glBufferData': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(257): error C3861: 'glEnableVertexAttribArray': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(258): error C3861: 'glVertexAttribPointer': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(265): error C3861: 'glEnableVertexAttribArray': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(266): error C3861: 'glVertexAttribPointer': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(274): error C3861: 'glGenFramebuffers': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(292): error C3861: 'glBindFramebuffer': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(293): error C3861: 'glFramebufferTexture2D': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(295): error C3861: 'glBindFramebuffer': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(306): error C3861: 'glGenBuffers': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(307): error C3861: 'glBindBuffer': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(308): error C3861: 'glBufferData': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(309): error C3861: 'glBindBufferBase': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(310): error C3861: 'glBindBuffer': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(312): error C3861: 'glGenBuffers': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(313): error C3861: 'glBindBuffer': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(314): error C3861: 'glBufferData': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(315): error C3861: 'glBindBufferBase': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(316): error C3861: 'glBindBuffer': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(318): error C3861: 'glGenBuffers': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(319): error C3861: 'glBindBuffer': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(320): error C3861: 'glBufferData': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(321): error C3861: 'glBindBufferBase': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(322): error C3861: 'glBindBuffer': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(357): error C3861: 'glGetUniformLocation': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(358): error C3861: 'glUniformMatrix4fv': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(361): error C3861: 'glGetUniformLocation': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(362): error C3861: 'glUniformMatrix4fv': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(367): error C3861: 'glGetUniformLocation': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(368): error C3861: 'glUniformMatrix4fv': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(370): error C3861: 'glGetUniformLocation': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(371): error C3861: 'glUniformMatrix4fv': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(375): error C3861: 'glUniformBlockBinding': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(379): error C3861: 'glUniformBlockBinding': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(383): error C3861: 'glUniformBlockBinding': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(392): error C3861: 'glBindBuffer': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(393): error C3861: 'glBufferData': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(394): error C3861: 'glMapBuffer': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(417): error C3861: 'glUnmapBuffer': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(418): error C3861: 'glBindBuffer': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(423): error C3861: 'glBindBuffer': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(424): error C3861: 'glBufferData': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(425): error C3861: 'glMapBuffer': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(437): error C3861: 'glUnmapBuffer': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(438): error C3861: 'glBindBuffer': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(443): error C3861: 'glBindBuffer': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(444): error C3861: 'glBufferData': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(445): error C3861: 'glMapBuffer': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(456): error C3861: 'glUnmapBuffer': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(457): error C3861: 'glBindBuffer': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(510): error C3861: 'glBlendEquation': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(520): error C3861: 'glBlendEquation': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(530): error C3861: 'glUseProgram': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(563): error C3861: 'glBindFramebuffer': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(596): error C3861: 'glBindBuffer': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(598): error C3861: 'glEnableVertexAttribArray': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(599): error C3861: 'glEnableVertexAttribArray': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(600): error C3861: 'glDisableVertexAttribArray': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(601): error C3861: 'glDisableVertexAttribArray': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(603): error C3861: 'glEnableVertexAttribArray': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(604): error C3861: 'glVertexAttribPointer': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(611): error C3861: 'glEnableVertexAttribArray': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(612): error C3861: 'glVertexAttribPointer': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(646): error C3861: 'glActiveTexture': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(652): error C3861: 'glGetUniformLocation': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(653): error C3861: 'glUniformMatrix4fv': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(655): error C3861: 'glGetUniformLocation': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(656): error C3861: 'glUniformMatrix4fv': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(658): error C3861: 'glGetUniformLocation': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(659): error C3861: 'glUniformMatrix4fv': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(661): error C3861: 'glGetUniformLocation': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(662): error C3861: 'glUniformMatrix4fv': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(664): error C3861: 'glGetUniformLocation': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(665): error C3861: 'glProgramUniform1i': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(669): error C3861: 'glUniformBlockBinding': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(673): error C3861: 'glUniformBlockBinding': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(677): error C3861: 'glUniformBlockBinding': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(692): error C3861: 'glActiveTexture': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(698): error C3861: 'glGetUniformLocation': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(699): error C3861: 'glUniformMatrix4fv': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(701): error C3861: 'glGetUniformLocation': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(702): error C3861: 'glUniformMatrix4fv': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(704): error C3861: 'glGetUniformLocation': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(705): error C3861: 'glUniformMatrix4fv': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(707): error C3861: 'glGetUniformLocation': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(708): error C3861: 'glUniformMatrix4fv': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(710): error C3861: 'glGetUniformLocation': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(711): error C3861: 'glProgramUniform1i': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(715): error C3861: 'glUniformBlockBinding': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(719): error C3861: 'glUniformBlockBinding': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(723): error C3861: 'glUniformBlockBinding': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(754): error C3861: 'glBindFramebuffer': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(765): error C3861: 'glCheckFramebufferStatus': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(768): error C3861: 'glBindFramebuffer': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(786): error C3861: 'glGetUniformLocation': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(787): error C3861: 'glUniformMatrix4fv': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(789): error C3861: 'glActiveTexture': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(789): fatal error C1003: error count exceeds 100; stopping compilation
1>  Material.cpp
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\material.cpp(62): error C3861: 'glActiveTexture': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\material.cpp(69): error C3861: 'glGetUniformLocation': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\material.cpp(70): error C3861: 'glProgramUniform1i': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\material.cpp(72): error C3861: 'glGetUniformLocation': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\material.cpp(73): error C3861: 'glProgramUniform4f': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\material.cpp(79): error C3861: 'glGetUniformLocation': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\material.cpp(88): error C3861: 'glUniform1f': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\material.cpp(93): error C3861: 'glUniform4f': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\material.cpp(101): error C3861: 'glActiveTexture': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\material.cpp(103): error C3861: 'glProgramUniform1i': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\material.cpp(124): warning C4996: 'strncpy': This function or variable may be unsafe. Consider using strncpy_s instead. To disable deprecation, use _CRT_SECURE_NO_WARNINGS. See online help for details.
1>  c:\program files (x86)\windows kits\10\include\10.0.10240.0\ucrt\string.h(346): note: see declaration of 'strncpy'
1>  Asset.cpp
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\asset.cpp(231): warning C4661: 'void SimpleAssetManager<Shader>::CheckInvariant(void)': no suitable definition provided for explicit template instantiation request
1>  c:\users\jvalenzu\source\2dvolumetriclighting\render\asset.h(190): note: see declaration of 'SimpleAssetManager<Shader>::CheckInvariant'
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\asset.cpp(232): warning C4661: 'void SimpleAssetManager<Texture>::CheckInvariant(void)': no suitable definition provided for explicit template instantiation request
1>  c:\users\jvalenzu\source\2dvolumetriclighting\render\asset.h(190): note: see declaration of 'SimpleAssetManager<Texture>::CheckInvariant'
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\asset.cpp(178): warning C4661: 'void SimpleAssetManager<Shader>::CheckInvariant(void)': no suitable definition provided for explicit template instantiation request
1>  c:\users\jvalenzu\source\2dvolumetriclighting\render\asset.h(190): note: see declaration of 'SimpleAssetManager<Shader>::CheckInvariant'
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\asset.cpp(178): warning C4661: 'void SimpleAssetManager<Texture>::CheckInvariant(void)': no suitable definition provided for explicit template instantiation request
1>  c:\users\jvalenzu\source\2dvolumetriclighting\render\asset.h(190): note: see declaration of 'SimpleAssetManager<Texture>::CheckInvariant'
1>  Generating Code...
========== Build: 0 succeeded, 1 failed, 0 up-to-date, 0 skipped ==========
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(870): error C3861: 'glDeleteBuffers': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(871): error C3861: 'glDeleteBuffers': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(872): error C3861: 'glDeleteVertexArrays': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\material.cpp(74): error C3861: 'glUniform1ui': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(755): error C3861: 'glGetActiveUniform': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\render\render.cpp(792): error C3861: 'glUniform1i': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\engine\debugui.cpp(109): error C3861: 'glBlendEquationSeparate': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\engine\debugui.cpp(237): error C3861: 'glGetAttribLocation': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\engine\debugui.cpp(272): error C3861: 'glDetachShader': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\engine\debugui.cpp(273): error C3861: 'glDeleteShader': identifier not found
1>c:\users\jvalenzu\source\2dvolumetriclighting\engine\debugui.cpp(280): error C3861: 'glDeleteProgram': identifier not found
