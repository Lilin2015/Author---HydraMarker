classdef Cprocs_mngr < handle
    %CPROCS_MNGR æ­¤å??¾ç¤º???³æ?¤ç±»????è¦?
    %   æ­¤å??¾ç¤ºè¯?ç»?è¯´æ??
    
    properties (SetAccess=private)
        tree
        tree_record
        full_record

        unum
        tree_iter
        tree_depth
        
        max_iter
        iter
    end
    
    methods
        
        function assign(obj,unum,max_iter)
            mustBeInteger(unum);
            %mustBeInteger(max_iter);
            assert(unum>0,'unum must be positive!');
            assert(max_iter>unum,'max_iter must be larger than unum!');
            
            obj.tree = cell(unum,1);
            obj.tree_record = -nan(unum,2);
            obj.full_record = -nan(max_iter,2);
            
            obj.unum = unum;
            obj.tree_iter = 0;
            obj.tree_depth = 0;
            obj.max_iter = max_iter;
            obj.iter = 0;
        end
        
        function flag = is_end(obj)
            flag = false;
            if obj.iter > obj.max_iter
                flag = true;
            end
        end % function
        
        function [msg,sugst] = procs_sugst(obj,msg,data)
            %mustBeA(msg,'Dmsg');
            sugst = [];
            
            if strcmp(msg.str,'empty')
                
            elseif strcmp(msg.str,'tag_repeated')
                [msg,sugst] = obj.roll_back;
                
            elseif strcmp(msg.str,'state_complete')
                msg = Dmsg('procs_complete');
                
            elseif strcmp(msg.str,'s_state_no_support')
                [msg,sugst] = obj.roll_back;
                
            elseif strcmp(msg.str,'filling_sugst')
                if obj.tree_iter == obj.tree_depth
                    [msg,sugst] = obj.depth_forward(data);
                elseif obj.tree_iter < obj.tree_depth
                    [msg,sugst] = obj.another_choice();
                end
                
            end        
            
        end % function

    end % method
    
    methods (Access=private)
        
        function [msg,sugst] = roll_back(obj)
            
            back_step = 1;
            iter_org = obj.tree_iter;
            while isempty(obj.tree{obj.tree_iter})
                back_step = back_step + 1;
                obj.tree_iter = obj.tree_iter - 1;    
            end
            obj.tree_iter = obj.tree_iter - 1;    
            
            msg = Dmsg('modify');
            sugst.ind = obj.tree_record(iter_org-back_step+1:iter_org,1);
            sugst.val = -ones(size(sugst.ind,1),1);
            obj.tree_record(iter_org-back_step+1:iter_org,:) = [];
            
            obj.full_record(obj.iter+1:obj.iter+back_step,:) = [sugst.ind, sugst.val];
            obj.iter = obj.iter + back_step;
        end
        
        function [msg,sugst] = depth_forward(obj,data)
            obj.tree_iter = obj.tree_iter + 1;
            
            % only consider likelihood sugst
            obj.tree{obj.tree_iter} = [data.ind,data.val];
%             obj.tree{obj.tree_iter} = [data.ind,data.val;
%                                        data.ind,~data.val;];
            msg = Dmsg('modify');
            sugst.ind = obj.tree{obj.tree_iter}(1,1);
            sugst.val = obj.tree{obj.tree_iter}(1,2);

            obj.tree_record(obj.tree_iter,:) = [sugst.ind,sugst.val];
            obj.tree{obj.tree_iter}(1,:) = [];
            
            obj.iter = obj.iter + 1;
            obj.full_record(obj.iter,:) = [sugst.ind,sugst.val];
            
            obj.tree_depth = obj.tree_iter;
            
        end
        
        function [msg,sugst] = another_choice(obj)
            
            if isempty(obj.tree{obj.tree_iter+1})
                [msg,sugst] = obj.roll_back;
                return
            end
            
            obj.tree_iter = obj.tree_iter + 1;
            
            msg = Dmsg('modify');
            sugst.ind = obj.tree{obj.tree_iter}(1,1);
            sugst.val = obj.tree{obj.tree_iter}(1,2);
            
            obj.tree_record(obj.tree_iter,:) = [sugst.ind,sugst.val];
            obj.tree{obj.tree_iter}(1,:) = [];
            
            obj.iter = obj.iter + 1;
            obj.full_record(obj.iter,:) = [sugst.ind,sugst.val];
            
            obj.tree_depth = obj.tree_iter;
        end
        
    end % method
end

