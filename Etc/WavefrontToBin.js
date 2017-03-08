var fs = require('fs');

var options = {
    globalVertices: false
};

fs.readFile('rooster.obj', 'utf8', function (err, data)
            {
                let vector = function (x,y,z)
                {
                    this.x = x;
                    this.y = y;
                    this.z = z;
                    return this;
                };
                
                let vecSub = function (v0, v1)
                {
                    let x0 = v0.position[0];
                    let x1 = v1.position[0];
                    let y0 = v0.position[1];
                    let y1 = v1.position[1];
                    let z0 = v0.position[2];
                    let z1 = v1.position[2];
                    return new vector(x0 - x1, y0 - y1, z0 - z1);
                };
                
                let vecDot = function(v0, v1)
                {
                    let s = 0.0;
                    s += v0.x*v1.x;
                    s += v0.y*v1.y;
                    s += v0.z*v1.z;
                    return s;
                };
                
                let vecLength = function (v0)
                {
                    let x = vecDot(v0, v0);
                    return Math.sqrt(x);
                };
                
                let lines = data.split("\n");
                let vp = [];
                let vn = [];
                let vt = [];
                let numMaterials = 0;
                let materials = [];
                
                // first do pass to collect vertices and count unique materials
                while (lines.length)
                {
                    let line = lines.shift().split('\r')[0];
                    
                    if (line.indexOf("usemtl ") == 0)
                        numMaterials++;
                    
                    if (line.indexOf("mtllib ") == 0)
                    {
                        let linearray = line.split(" ");
                        let mtldata = fs.readFileSync(linearray[1].split("\r")[0], 'utf8');
                        let mlines = mtldata.split("\n");
                        let material = undefined;
                        let materialName = undefined;
                        while (mlines.length)
                        {
                            let mline = mlines.shift().split('\r')[0];
                            
                            if (mline.indexOf('#') == 0)
                                continue;
                            if (mline.length == 0)
                                continue;
                            
                            if (mline.indexOf("newmtl ") == 0)
                            {
                                let mlinearray = mline.split(" ");
                                materialName = mlinearray[1];
                                material = materials[materialName] = {};
                                material.Ks = [ 0, 0, 0 ];
                                material.Ke = [ 0, 0, 0 ];
                                material.Ka = [ 0, 0, 0 ];
                                material.Kd = [ 0, 0, 0 ];
                                material.Ns = 0.0;
                                material.Ni = 0.0;
                                material.d = 0.0;
                            }
                            else
                            {
                                let mlinearray = mline.split(" ");
                                let name = mlinearray.shift();
                                
                                for (let i=0; i<mlinearray.length; ++i)
                                {
                                    if (Number.parseFloat(mlinearray[i]) != NaN)
                                        mlinearray[i] = Number.parseFloat(mlinearray[i]);
                                }
                                
                                if (mlinearray.length == 1)
                                {
                                    materials[materialName][name] = mlinearray[0];
                                }
                                else
                                {
                                    materials[materialName][name] = mlinearray;
                                }
                            }
                        }
                    }
                    
                    if (line.indexOf("v ") == 0)
                    {
                        let linearray = line.split(" ").map(Number.parseFloat);
                        vp[vp.length] = [ linearray[1], linearray[2], linearray[3] ];
                    }
                    
                    if (line.indexOf("vn ") == 0)
                    {
                        let linearray = line.split(" ").map(Number.parseFloat);;
                        vn[vn.length] = [ linearray[1], linearray[2], linearray[3] ];
                    }
                    
                    if (line.indexOf("vt ") == 0)
                    {
                        let linearray = line.split(" ").map(Number.parseFloat);;
                        vt[vt.length] = [ linearray[1], linearray[2]];
                    }
                }

                let surfaces = [];
                let surface = undefined;
                let vert_hash = {};
                let vertices = [];
                let polyname = undefined;
                
                // now
                lines = data.split("\n");
                while (lines.length)
                {
                    let line = lines.shift().split('\r')[0];
                    if (line.indexOf("o ") == 0)
                    {
                        let linearray = line.split(" ");
                        polyname = linearray[1].split("\r")[0];
                    }

                    if (line.indexOf("usemtl ") == 0)
                    {
                        let linearray = line.split(" ");
                        surface = { name: polyname + "." + surfaces.length, material: linearray[1], vertices: vertices, indices: [] };
                        surfaces[surfaces.length] = surface;
                        
                        if (!options.globalVertices)
                        {
                            // WaveFront format requires global vertices, that is, a later surface can
                            // have an index to a vertex that was defined for a previous surface.
                            surface.vertices = [];
                            vert_hash = {};
                        }
                        continue;
                    }
                    
                    if (line.indexOf("f ") == 0)
                    {
                        let getVertex = function(key)
                        {
                            let vertValues = key.split('/');
                            let pi = vertValues[0];
                            let ti = vertValues[1];
                            let ni = vertValues[2];
                            
                            return {
                                position: vp[pi-1],
                                texcoord: vt[ti-1],
                                normal:   vn[ni-1]
                            };
                        };
                        
                        let addface = function(linearray)
                        {
                            let nsert = function (key)
                            {
                                if (key in vert_hash)
                                {
                                    return;
                                }
                                
                                let vertValues = key.split('/');
                                let pi = vertValues[0];
                                let ti = vertValues[1];
                                let ni = vertValues[2];
                                
                                let vertex = {
                                    position: vp[pi-1],
                                    texcoord: vt[ti-1],
                                    normal:   vn[ni-1]
                                };
                                
                                if (ti == "")
                                    delete vertex.texcoord;
                                
                                let vi = surface.vertices.length;
                                surface.vertices[vi] = vertex;
                                vert_hash[key] = vi;
                            };
                            
                            // lookup or add add indices for these vertexes.  If we've seen this
                            // position/tangent/normal combination before, lookup.  If we haven't,
                            // insert.
                            nsert(linearray[0]);
                            nsert(linearray[1]);
                            nsert(linearray[2]);
                            
                            // whether we had to insert them or not, they are present now, so
                            // add the indices for these triangles
                            surface.indices[surface.indices.length] = vert_hash[linearray[0]];
                            surface.indices[surface.indices.length] = vert_hash[linearray[1]];
                            surface.indices[surface.indices.length] = vert_hash[linearray[2]];
                        };
                        
                        let linearray = line.split(" ");
                        linearray.shift();
                        if (linearray.length == 3)
                        {
                            addface(linearray);
                        }
                        else
                        {
                            // very ghetto triangulation
                            for (let i=0; i<linearray.length; ++i)
                            {
                                let i_p1 = i - 1;
                                if (i_p1 < 0)
                                    i_p1 = linearray.length-1;
                                let i_n1 = (i+1)%linearray.length;
                                
                                let tri1 = [ linearray[i_p1], linearray[i], linearray[i_n1] ];
                                addface(tri1);
                            }
                        }
                    };
                }
                
                let kSimpleVertexSize = 12+12+6+4;
                let offset = 0;
                
                offset += 4; // number of surfaces
                
                for (let surfaceIndex in surfaces)
                {
                    let surface = surfaces[surfaceIndex];
                    
                    offset += 2;
                    offset += 2;

                    // material
                    offset += 15*4;
                    
                    offset += (surface.vertices.length * (12 + 12 + 8 + 4));
                    offset += (surface.indices.length * 2);
                }
                
                let buffer = Buffer.allocUnsafe(offset);
                
                let itr = 0;
                buffer.writeUInt16LE(surfaces.length, itr);
                itr += 2;
                itr += 2; // padding
                
                for (let surfaceIndex in surfaces)
                {
                    let surface = surfaces[surfaceIndex];

                    buffer.writeUInt16LE(surface.vertices.length, itr);
                    itr += 2;
                    
                    buffer.writeUInt16LE(surface.indices.length, itr);
                    itr += 2;
                    
                    let material = materials[surface.material];
                    buffer.writeFloatLE(material.Ns, itr);           itr += 4;
                    buffer.writeFloatLE(material.Ka[0], itr);        itr += 4;
                    buffer.writeFloatLE(material.Ka[1], itr);        itr += 4;
                    buffer.writeFloatLE(material.Ka[2], itr);        itr += 4;
                    buffer.writeFloatLE(material.Kd[0], itr);        itr += 4;
                    buffer.writeFloatLE(material.Kd[1], itr);        itr += 4;
                    buffer.writeFloatLE(material.Kd[2], itr);        itr += 4;
                    buffer.writeFloatLE(material.Ks[0], itr);        itr += 4;
                    buffer.writeFloatLE(material.Ks[1], itr);        itr += 4;
                    buffer.writeFloatLE(material.Ks[2], itr);        itr += 4;
                    buffer.writeFloatLE(material.Ke[0], itr);        itr += 4;
                    buffer.writeFloatLE(material.Ke[1], itr);        itr += 4;
                    buffer.writeFloatLE(material.Ke[2], itr);        itr += 4;
                    buffer.writeFloatLE(material.Ni, itr);           itr += 4;
                    buffer.writeFloatLE(material.d, itr);            itr += 4;
                    
                    for (let i=0; i<surface.vertices.length; ++i)
                    {
                        let vertex = surface.vertices[i];
                        
                        buffer.writeFloatLE(vertex.position[0], itr);
                        itr += 4;
                        
                        buffer.writeFloatLE(vertex.position[1], itr);
                        itr += 4;
                        
                        buffer.writeFloatLE(vertex.position[2], itr);
                        itr += 4;
                        
                        if (!("normal" in vertex))
                        {
                            throw vertex.position;
                        }
                        
                        buffer.writeFloatLE(vertex.normal[0], itr);
                        itr += 4;
                        
                        buffer.writeFloatLE(vertex.normal[1], itr);
                        itr += 4;
                        
                        buffer.writeFloatLE(vertex.normal[2], itr);
                        itr += 4;
                        
                        if ("texcoord" in vertex)
                        {
                            buffer.writeFloatLE(vertex.texcoord[0], itr);
                            itr += 4;
                            
                            buffer.writeFloatLE(vertex.texcoord[1], itr);
                            itr += 4;
                        }
                        else
                        {
                            buffer.writeFloatLE(0.0, itr);
                            itr += 4;
                            
                            buffer.writeFloatLE(0.0, itr);
                            itr += 4;
                        }
                        buffer.writeUInt32LE(0xffffffff, itr);
                        itr += 4;
                    }
                    
                    for (let i=0; i<surface.indices.length; ++i)
                    {
                        buffer.writeUInt16LE(surface.indices[i], itr);
                        itr += 2;
                    }
                }
                
                // build output also in blender format, but with combined positions/normals
                // var s = "";
                // 
                // s += "# Blender v2.78 (sub 0) OBJ File: 'foo.blend'\n";
                // s += "# www.blender.org\n";
                // s += "mtllib bar.mtl\n";
                // 
                // for (let surfaceIndex in surfaces)
                // {
                //     let surface = surfaces[surfaceIndex];
                //     
                //     s += "o " + surface.name + "\n";
                //     
                //     for (let i=0; i<surface.vertices.length; ++i)
                //     {
                //         let vertex = surface.vertices[i];
                //         s += "v " + vertex.position[0] + " " + vertex.position[1] + " " + vertex.position[2] + "\n";
                //     }
                //     for (let i=0; i<surface.vertices.length; ++i)
                //     {
                //         let vertex = surface.vertices[i];
                //         s += "vn " + vertex.normal[0] + " " + vertex.normal[1] + " " + vertex.normal[2] + "\n";
                //     }
                //     
                //     s += "usemtl Blue04\n";
                //     
                //     for (let i=0; i<surface.indices.length; i += 3)
                //     {
                //         var index0 = surface.indices[i+0]+1;
                //         var index1 = surface.indices[i+1]+1;
                //         var index2 = surface.indices[i+2]+1;
                //         s += "f " + index0 + "//" + index0 + " " + index1 + "//" + index1 + " " + index2 + "//" + index2 + "\n";
                //     }
                // }
                // fs.writeFile("Rooster.obj", s);
                fs.writeFile("Rooster.bin", buffer);
                // fs.writeFile("Rooster.json", JSON.stringify(surfaces, null, 2));
            });
