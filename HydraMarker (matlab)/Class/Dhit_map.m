classdef Dhit_map
    
    properties
        val
        %   val is a cell.
        %   Each cell is a matrix with multiple slices to indicate which tag hits the element.
        %   0-miss; 1-hit; -2-hit a hollow.
        %   For example, val{2,3}(5,8,3)=1 means the tag in pool{2,3}(:,:,3)
        %   hits the field at (5,8) (top-left);
    end
    
    methods
        
        function obj = Dhit_map(hit_map)
            mustBeNonempty(hit_map);
            assert(iscell(hit_map),'hit_map must be a cell!');
            assert(size(size(hit_map),2)==2 && size(hit_map,1)>0 && size(hit_map,2)==4,'hit_map must be a Nx4 cell!');
            for t = size(hit_map,1)*4
                mustBeMember(hit_map{t},[0,1,-1,-2]);
            end
            obj.val = hit_map;
        end
        
    end
end

