classdef Dshape
% user-defined shape

    properties
        val
        rot_prop
        %   val is a cell. Each cell describes the shape of an ID tag by a binary matrix.
        %   For example, [0 1 0;1 1 1;0 1 0] describes a cross.
    end
    
    methods
        
        function obj = Dshape(shape)
            assert(size(size(shape),2)==2 && size(shape,1)>0 && size(shape,2)==1,'shape must be a Nx1 cell!');
            for it = 1 : size(shape,1)
                assert(size(size(shape{it}),2)==2 && size(shape{it},1)>0 && size(shape{it},2)>0,'the member of shape must be one layer matrix!');
                mustBeNonempty(shape{it});
                mustBeMember(shape{it},[0,1,-1]);
                assert(all(sum(shape{it},2)>0,'all'),'shape has a zero row!');
                assert(all(sum(shape{it},1)>0,'all'),'shape has a zero column!');
            end
            obj.val = shape;
            
            for it = 1 : size(shape,1)
                [shape_M,shape_N] = size(shape{it});
                if (shape_M == shape_N) && all(all(shape{it}==rot90(shape{it})))
                    obj.rot_prop{it} = [1,2,3,4];
                elseif all(all(shape{it}==rot90(shape{it},2)))
                    obj.rot_prop{it} = [1,3;2,4];
                else
                    obj.rot_prop{it} = [1;2;3;4];
                end   
            end
            
        end
        
    end % end method
end

