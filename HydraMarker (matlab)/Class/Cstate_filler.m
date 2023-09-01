classdef Cstate_filler < handle
    
    methods (Static)
        
        function sugst = fill(s_state)
            mustBeA(s_state,'Dsuperstate');
            
            % find the elements with most unbalance support
            s_state.support(s_state.support<0) = Inf;
            minSize = min(s_state.support(:));
            if isinf(minSize)
                sugst.msg = 'complete';
                return;
            elseif minSize == 0
                sugst.msg = 'no_support';
                return;
            end
            ind = find(s_state.support==minSize);
            % find the one with most biased possibilities
            imba = nan(size(s_state.likelihood));
            imba(ind) = abs(s_state.likelihood(ind)-0.5);
            ind = find(imba==max(imba(:)));
            sugst.ind = ind(1);
            % collapse to the value with larger possibility
            if s_state.likelihood(ind) <= 0.5
                sugst.val = 0;
            else
                sugst.val = 1;
            end
            sugst.msg = 'success';
            
        end % function
        
    end % method
end

