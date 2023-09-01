classdef Dfield
% marker field
    
    properties
        val % a matrix with 4 states: 0, 1, -1 unknown, -2 hollow
    end
    
    methods
        
        function obj = Dfield(state)
            mustBeInteger(state);
            assert(ismatrix(state),'marker field must be a matrix!');
            mustBeNonempty(state);
            %mustBeMember(state,[0,1,-1,-2]);
            obj.val = state;
        end
        
    end % method
end

