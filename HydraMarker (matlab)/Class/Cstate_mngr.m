classdef Cstate_mngr < handle
    
    properties (SetAccess=private)
        pool Dpool
    end
    
    properties (Access=private)
        pool_org Dpool
    end
    
    methods
        function assign(obj,shape)
            %mustBeA(shape,'Dshape');
            obj.pool_org = Dpool(shape);
            obj.pool = obj.pool_org;
        end
        
        function [msg,s_state] = update(obj,state)
           %mustBeA(state,'Dfield');
           [msg,obj.pool] = Cpool_mngr.remove_exist(obj.pool_org,state);
           if strcmp(msg.str,'tag_repeated')
               s_state = [];
               return;
           end
           s_state = Csuperstate_mngr.calc_super_state(obj.pool,state);
        end
    end
    
    methods (Static)
        function [msg,sugst] = filling_sugst(s_state,flag)
            %mustBeA(s_state,'Dsuperstate');
            if ~exist('flag','var')
                flag = 's-first';
            end
            %% bWFC, Re_CL
            if strcmp(flag,'p-first')
                sugst = [];
                % find the elements with most unbalance support
                if ismember(-3,s_state.support)
                    msg = Dmsg('s_state_no_support');
                    return;
                end
                s_state.support(s_state.support<0) = Inf;
                minSize = min(s_state.support(:));
                if isinf(minSize)
                    msg = Dmsg('state_complete');
                    return;
                end
                % find the elements with most unbalance support
                ind = find(s_state.support==minSize);
                % find the one with most biased possibilities
                imba = nan(size(s_state.likelihood));
                imba(ind) = abs(s_state.likelihood(ind)-0.5);
                sugst.ind = find(imba==max(imba(:)));
                % collapse to the value with larger possibility
                sugst.val = ~(s_state.likelihood(sugst.ind) <= 0.5);
                sugst.ind = [sugst.ind;sugst.ind];
                sugst.val = [sugst.val;~sugst.val];
                msg = Dmsg('filling_sugst');
            % bWFC, state-?rst recti?cation
            elseif strcmp(flag,'s-first')
                sugst = [];
                if ismember(-3,s_state.support)
                    msg = Dmsg('s_state_no_support');
                    return;
                end
                s_state.support(s_state.support<0) = Inf;
                minSize = min(s_state.support(:));
                if isinf(minSize)
                    msg = Dmsg('state_complete');
                    return;
                end
                % find the elements with most unbalance support
                ind = find(s_state.support==minSize);
                % find the one with most biased possibilities
                imba = nan(size(s_state.likelihood));
                imba(ind) = abs(s_state.likelihood(ind)-0.5);
                sugst.ind = find(imba==max(imba(:)));
                % collapse to the value with larger possibility
                sugst.val = ~(s_state.likelihood(sugst.ind) <= 0.5);
                sugst.val = [sugst.val,~sugst.val]';
                sugst.val = sugst.val(:);
                sugst.ind = [sugst.ind,sugst.ind]';
                sugst.ind = sugst.ind(:);
                
                msg = Dmsg('filling_sugst');
            % bWFC, support only
            elseif strcmp(flag,'c-only')
                sugst = [];
                if ismember(-3,s_state.support)
                    msg = Dmsg('s_state_no_support');
                    return;
                end
                s_state.support(s_state.support<0) = Inf;
                minSize = min(s_state.support(:));
                if isinf(minSize)
                    msg = Dmsg('state_complete');
                    return;
                end
                % find the elements with most unbalance support
                ind_1 = find(s_state.support==minSize&s_state.likelihood==1);
                ind_0 = find(s_state.support==minSize&s_state.likelihood==0);
                ind_e = find(s_state.support==minSize&s_state.likelihood>0&s_state.likelihood<1);
                
                sugst.ind = [ind_0;ind_1;ind_e;ind_e];
                sugst.val = [zeros(size(ind_0));ones(size(ind_1));zeros(size(ind_e));ones(size(ind_e))];
                
                msg = Dmsg('filling_sugst');
            % bWFC, likelihood only
            elseif strcmp(flag,'l-only')
                sugst = [];
                if ismember(-3,s_state.support)
                    msg = Dmsg('s_state_no_support');
                    return;
                end
                s_state.support(s_state.support<0) = Inf;
                minSize = min(s_state.support(:));
                if isinf(minSize)
                    msg = Dmsg('state_complete');
                    return;
                end
                % find the elements with most unbalance likelihood
                legal = find(s_state.likelihood>=0&~isinf(s_state.support)&s_state.support>=0);
                L = abs(0.5-s_state.likelihood);
                maxSize = max(L(legal));
                
                sugst.ind = find(L==maxSize&~isinf(s_state.support)&s_state.support>=0);
                % collapse to the value with larger possibility
                sugst.val = ~(s_state.likelihood(sugst.ind) <= 0.5);
                sugst.val = [sugst.val,~sugst.val]';
                sugst.val = sugst.val(:);
                sugst.ind = [sugst.ind,sugst.ind]';
                sugst.ind = sugst.ind(:);
                msg = Dmsg('filling_sugst');
            end
        end % function
        
    end % method
end

