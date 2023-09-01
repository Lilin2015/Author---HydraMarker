classdef Dsuperstate
% superstate of marker field
    
    properties
        likelihood % likelihood describes the possibilities of unknown elements being 1;
        support % support means the min support number of 0/1 from nearby stack;
        %   For known states, L and C are -1; For hollows, they are -2
        %   For the states that be neither 0 nor 1, they are -3;
    end
    
    methods
        
        function obj = Dsuperstate(likelihood,support)
            mustBeNonempty(likelihood);
            assert(ismatrix(likelihood),'likelihood must be a matrix!');
            assert(all(ismember(likelihood,[-1,-2,-3]) | likelihood>=0 & likelihood<=1,'all'),'the range of likelihood must be [0 1], or -1, -2, -3!');
            
            mustBeNonempty(support);
            assert(ismatrix(support),'support must be a matrix!');
            assert(all(ismember(support,[-1,-2,-3]) | support>=0,'all'),'the value of support must be no smaller than 0, or -1, -2, -3!');
            
            obj.likelihood = likelihood;
            obj.support = support;
        end
        
    end
end

